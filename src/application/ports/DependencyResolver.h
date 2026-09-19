#pragma once

#include "domain/dependency/ResolvedDependency.h"

#include <string>

class DependencyResolver
{
public:
    virtual ~DependencyResolver() = default;

    virtual ResolvedDependency resolveBySearchTerm(
        const std::string &term,
        const std::string &version) const = 0;

    virtual ResolvedDependency resolveByCoordinate(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version) const = 0;
};