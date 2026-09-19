#include "application/errors/MultipleDependencyMatches.h"

#include <utility>

MultipleDependencyMatches::MultipleDependencyMatches(
    std::string query,
    std::vector<ResolvedDependency> candidates,
    std::size_t dependencyIndex)
    : DependencyResolutionError("Multiple dependency matches found: " + query),
      query_(std::move(query)),
      candidates_(std::move(candidates)),
      dependencyIndex_(dependencyIndex)
{
}

const std::string &MultipleDependencyMatches::query() const
{
    return query_;
}

const std::vector<ResolvedDependency> &MultipleDependencyMatches::candidates() const
{
    return candidates_;
}

std::size_t MultipleDependencyMatches::dependencyIndex() const
{
    return dependencyIndex_;
}
