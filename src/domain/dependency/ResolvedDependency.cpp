#include "domain/dependency/ResolvedDependency.h"

#include <utility>

ResolvedDependency::ResolvedDependency(
    std::string groupId,
    std::string artifactId,
    std::string version,
    std::string scope)
    : groupId_(std::move(groupId)),
      artifactId_(std::move(artifactId)),
      version_(std::move(version)),
      scope_(std::move(scope))
{
}

const std::string &ResolvedDependency::groupId() const
{
    return groupId_;
}

const std::string &ResolvedDependency::artifactId() const
{
    return artifactId_;
}

const std::string &ResolvedDependency::version() const
{
    return version_;
}

const std::string &ResolvedDependency::scope() const
{
    return scope_;
}
