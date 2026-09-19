#include "application/errors/DependencyResolutionError.h"

DependencyResolutionError::DependencyResolutionError(const std::string &message)
    : std::runtime_error(message)
{
}
