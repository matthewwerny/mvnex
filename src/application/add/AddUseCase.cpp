#include "application/add/AddUseCase.h"

#include "domain/dependency/ResolvedDependency.h"
#include "application/errors/MultipleDependencyMatches.h"
#include "application/ports/DependencyResolver.h"
#include "application/ports/MavenProjectValidator.h"
#include "application/ports/ProjectDependencyRepository.h"

#include <unordered_set>
#include <vector>

namespace
{
ResolvedDependency withRequestedScope(
    const ResolvedDependency &resolved,
    const DependencyRequest &request)
{
    return ResolvedDependency(
        resolved.groupId(),
        resolved.artifactId(),
        resolved.version(),
        request.scope());
}
}

AddUseCase::AddUseCase(
    const DependencyResolver &dependencyResolver,
    const ProjectDependencyRepository &projectDependencyRepository,
    const MavenProjectValidator &mavenProjectValidator) : dependencyResolver_(dependencyResolver),
                                                          projectDependencyRepository_(projectDependencyRepository),
                                                          mavenProjectValidator_(mavenProjectValidator)
{
}

AddUseCaseResponse AddUseCase::execute(const AddUseCaseRequest &request) const
{
    std::vector<ResolvedDependency> resolvedDependencies;
    std::vector<ResolvedDependency> dependenciesToAdd;
    std::vector<ResolvedDependency> skippedDependencies;

    for (std::size_t index = 0; index < request.dependencies.size(); ++index)
    {
        const DependencyRequest &dependency = request.dependencies[index];

        try
        {
            switch (dependency.type())
            {
            case DependencyRequestType::SearchTerm:
            case DependencyRequestType::SearchTermWithVersion:
            {
                ResolvedDependency resolved = dependencyResolver_.resolveBySearchTerm(dependency.query(), dependency.version());
                resolvedDependencies.push_back(withRequestedScope(resolved, dependency));
                break;
            }
            case DependencyRequestType::Coordinate:
            case DependencyRequestType::CoordinateWithVersion:
            {
                ResolvedDependency resolved = dependencyResolver_.resolveByCoordinate(dependency.groupId(), dependency.artifactId(), dependency.version());
                resolvedDependencies.push_back(withRequestedScope(resolved, dependency));
                break;
            }
            }
        }
        catch (const MultipleDependencyMatches &error)
        {
            throw MultipleDependencyMatches(
                error.query(),
                error.candidates(),
                index);
        }
    }

    std::unordered_set<std::string> seenDependencies;

    for (const ResolvedDependency &resolved : resolvedDependencies)
    {
        std::string key = resolved.groupId() + ":" + resolved.artifactId();

        if (projectDependencyRepository_.containsDependency(
                resolved.groupId(),
                resolved.artifactId()))
        {
            skippedDependencies.push_back(resolved);
            continue;
        }

        if (seenDependencies.contains(key))
        {
            skippedDependencies.push_back(resolved);
            continue;
        }

        seenDependencies.insert(key);
        dependenciesToAdd.push_back(resolved);
    }

    if (!dependenciesToAdd.empty())
    {
        projectDependencyRepository_.addDependencies(dependenciesToAdd);
    }

    MavenProjectValidationResult validationResult{
        MavenProjectValidationStatus::MavenNotFound};

    if (!dependenciesToAdd.empty())
    {
        validationResult = mavenProjectValidator_.validate();
    }

    return AddUseCaseResponse{
        dependenciesToAdd,
        skippedDependencies,
        validationResult};
}
