#include "application/errors/DependencyResolverUnavailable.h"

DependencyResolverUnavailable::DependencyResolverUnavailable(const std::string &message)
    : DependencyResolutionError(message)
{
}
