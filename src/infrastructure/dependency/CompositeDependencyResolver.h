#pragma once

#include "application/ports/DependencyResolver.h"

#include <functional>
#include <vector>

class CompositeDependencyResolver : public DependencyResolver
{
public:
    explicit CompositeDependencyResolver(
        std::vector<std::reference_wrapper<const DependencyResolver>> resolvers);

    ResolvedDependency resolveBySearchTerm(
        const std::string &term,
        const std::string &version) const override;

    ResolvedDependency resolveByCoordinate(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version) const override;

private:
    std::vector<std::reference_wrapper<const DependencyResolver>> resolvers_;
};