#include "infrastructure/dependency/CompositeDependencyResolver.h"

#include "application/errors/DependencyNotFound.h"
#include "application/errors/DependencyResolutionError.h"
#include "application/errors/DependencyResolverUnavailable.h"
#include "application/errors/MultipleDependencyMatches.h"

#include <chrono>
#include <future>
#include <optional>
#include <string>
#include <thread>
#include <utility>

namespace
{
enum class ResolutionFailureKind
{
    None,
    NotFound,
    Unavailable,
    Other
};

struct ResolutionFailure
{
    ResolutionFailureKind kind = ResolutionFailureKind::None;
    std::string message = "Dependency resolver failed.";
};

struct ResolutionAttempt
{
    std::optional<ResolvedDependency> dependency;
    std::optional<MultipleDependencyMatches> multipleMatches;
    ResolutionFailure failure;
};

ResolutionAttempt notFoundFailure(const DependencyNotFound &error)
{
    return ResolutionAttempt{
        std::nullopt,
        std::nullopt,
        ResolutionFailure{ResolutionFailureKind::NotFound, error.what()}};
}

ResolutionAttempt unavailableFailure(const DependencyResolverUnavailable &error)
{
    return ResolutionAttempt{
        std::nullopt,
        std::nullopt,
        ResolutionFailure{ResolutionFailureKind::Unavailable, error.what()}};
}

ResolutionAttempt genericFailure(const DependencyResolutionError &error)
{
    return ResolutionAttempt{
        std::nullopt,
        std::nullopt,
        ResolutionFailure{ResolutionFailureKind::Other, error.what()}};
}

ResolvedDependency resolveAttempts(
    std::vector<std::future<ResolutionAttempt>> &futures)
{
    std::optional<ResolutionFailure> notFound;
    std::optional<ResolutionFailure> unavailable;
    std::optional<ResolutionFailure> other;
    std::vector<bool> consumed(futures.size(), false);
    std::size_t remaining = futures.size();

    while (remaining > 0)
    {
        bool progressed = false;

        for (std::size_t i = 0; i < futures.size(); ++i)
        {
            if (consumed[i])
            {
                continue;
            }

            if (futures[i].wait_for(std::chrono::milliseconds(10)) !=
                std::future_status::ready)
            {
                continue;
            }

            consumed[i] = true;
            --remaining;
            progressed = true;

            ResolutionAttempt attempt = futures[i].get();

            if (attempt.dependency.has_value())
            {
                return attempt.dependency.value();
            }

            if (attempt.multipleMatches.has_value())
            {
                throw attempt.multipleMatches.value();
            }

            switch (attempt.failure.kind)
            {
            case ResolutionFailureKind::NotFound:
                notFound = attempt.failure;
                break;
            case ResolutionFailureKind::Unavailable:
                unavailable = attempt.failure;
                break;
            case ResolutionFailureKind::Other:
                other = attempt.failure;
                break;
            case ResolutionFailureKind::None:
                break;
            }
        }

        if (!progressed)
        {
            std::this_thread::yield();
        }
    }

    if (notFound.has_value())
    {
        throw DependencyResolutionError(notFound->message);
    }

    if (unavailable.has_value())
    {
        throw DependencyResolverUnavailable(unavailable->message);
    }

    if (other.has_value())
    {
        throw DependencyResolutionError(other->message);
    }

    throw DependencyResolutionError("Dependency resolver failed.");
}
}

CompositeDependencyResolver::CompositeDependencyResolver(
    std::vector<std::reference_wrapper<const DependencyResolver>> resolvers)
    : resolvers_(std::move(resolvers))
{
}

ResolvedDependency CompositeDependencyResolver::resolveBySearchTerm(
    const std::string &term,
    const std::string &version) const
{
    std::vector<std::future<ResolutionAttempt>> futures;
    futures.reserve(resolvers_.size());

    for (const DependencyResolver &resolver : resolvers_)
    {
        futures.push_back(std::async(
            std::launch::async,
            [&resolver, term, version]() -> ResolutionAttempt
            {
                try
                {
                    return ResolutionAttempt{
                        resolver.resolveBySearchTerm(term, version),
                        std::nullopt,
                        ResolutionFailure{}};
                }
                catch (const MultipleDependencyMatches &error)
                {
                    return ResolutionAttempt{
                        std::nullopt,
                        error,
                        ResolutionFailure{}};
                }
                catch (const DependencyNotFound &error)
                {
                    return notFoundFailure(error);
                }
                catch (const DependencyResolverUnavailable &error)
                {
                    return unavailableFailure(error);
                }
                catch (const DependencyResolutionError &error)
                {
                    return genericFailure(error);
                }
            }));
    }

    return resolveAttempts(futures);
}

ResolvedDependency CompositeDependencyResolver::resolveByCoordinate(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &version) const
{
    std::vector<std::future<ResolutionAttempt>> futures;
    futures.reserve(resolvers_.size());

    for (const DependencyResolver &resolver : resolvers_)
    {
        futures.push_back(std::async(
            std::launch::async,
            [&resolver, groupId, artifactId, version]() -> ResolutionAttempt
            {
                try
                {
                    return ResolutionAttempt{
                        resolver.resolveByCoordinate(groupId, artifactId, version),
                        std::nullopt,
                        ResolutionFailure{}};
                }
                catch (const MultipleDependencyMatches &error)
                {
                    return ResolutionAttempt{
                        std::nullopt,
                        error,
                        ResolutionFailure{}};
                }
                catch (const DependencyNotFound &error)
                {
                    return notFoundFailure(error);
                }
                catch (const DependencyResolverUnavailable &error)
                {
                    return unavailableFailure(error);
                }
                catch (const DependencyResolutionError &error)
                {
                    return genericFailure(error);
                }
            }));
    }

    return resolveAttempts(futures);
}
