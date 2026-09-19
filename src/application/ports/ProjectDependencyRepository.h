#pragma once

#include "domain/dependency/ResolvedDependency.h"

#include <string>
#include <vector>

class ProjectDependencyRepository
{
public:
    virtual ~ProjectDependencyRepository() = default;

    virtual bool projectFileExists() const = 0;

    virtual bool containsDependency(
        const std::string &groupId,
        const std::string &artifactId) const = 0;

    virtual void addDependencies(
        const std::vector<ResolvedDependency> &dependencies) const = 0;
};