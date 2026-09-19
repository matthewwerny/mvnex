#include "domain/dependency/DependencyRequest.h"

#include <utility>

DependencyRequest DependencyRequest::searchTerm(
    const std::string &query,
    const std::string &scope)
{
    return DependencyRequest(
        DependencyRequestType::SearchTerm,
        query,
        "",
        "",
        "",
        scope);
}

DependencyRequest DependencyRequest::searchTermWithVersion(
    const std::string &query,
    const std::string &version,
    const std::string &scope)
{
    return DependencyRequest(
        DependencyRequestType::SearchTermWithVersion,
        query,
        "",
        "",
        version,
        scope);
}

DependencyRequest DependencyRequest::coordinate(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &scope)
{
    return DependencyRequest(
        DependencyRequestType::Coordinate,
        "",
        groupId,
        artifactId,
        "",
        scope);
}

DependencyRequest DependencyRequest::coordinateWithVersion(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &version,
    const std::string &scope)
{
    return DependencyRequest(
        DependencyRequestType::CoordinateWithVersion,
        "",
        groupId,
        artifactId,
        version,
        scope);
}

DependencyRequestType DependencyRequest::type() const
{
    return type_;
}

const std::string &DependencyRequest::query() const
{
    return query_;
}

const std::string &DependencyRequest::groupId() const
{
    return groupId_;
}

const std::string &DependencyRequest::artifactId() const
{
    return artifactId_;
}

const std::string &DependencyRequest::version() const
{
    return version_;
}

const std::string &DependencyRequest::scope() const
{
    return scope_;
}

DependencyRequest::DependencyRequest(
    DependencyRequestType type,
    std::string query,
    std::string groupId,
    std::string artifactId,
    std::string version,
    std::string scope)
    : type_(type),
      query_(std::move(query)),
      groupId_(std::move(groupId)),
      artifactId_(std::move(artifactId)),
      version_(std::move(version)),
      scope_(std::move(scope))
{
}
