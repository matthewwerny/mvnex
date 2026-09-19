#include "application/errors/DependencyNotFound.h"

DependencyNotFound::DependencyNotFound(const std::string &query)
    : DependencyResolutionError("Dependency not found: " + query),
      query_(query)
{
}

const std::string &DependencyNotFound::query() const
{
    return query_;
}
