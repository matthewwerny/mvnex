#pragma once

#include "application/errors/DependencyResolutionError.h"
#include "domain/dependency/ResolvedDependency.h"

#include <string>
#include <utility>
#include <vector>

class MultipleDependencyMatches : public DependencyResolutionError
{
public:
    static constexpr std::size_t UnknownDependencyIndex = static_cast<std::size_t>(-1);

    MultipleDependencyMatches(
        std::string query,
        std::vector<ResolvedDependency> candidates,
        std::size_t dependencyIndex = UnknownDependencyIndex);

    const std::string &query() const;

    const std::vector<ResolvedDependency> &candidates() const;

    std::size_t dependencyIndex() const;

private:
    std::string query_;
    std::vector<ResolvedDependency> candidates_;
    std::size_t dependencyIndex_;
};
