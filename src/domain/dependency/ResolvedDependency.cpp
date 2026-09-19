#include "domain/dependency/ResolvedDependency.h"

#include <utility>

ResolvedDependency::ResolvedDependency(
    std::string groupId,
    std::string artifactId,
    std::string version)
    : groupId_(std::move(groupId)),
      artifactId_(std::move(artifactId)),
      version_(std::move(version))
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