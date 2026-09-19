#pragma once

#include "application/errors/DependencyResolutionError.h"

#include <string>

class DependencyResolverUnavailable : public DependencyResolutionError
{
public:
    explicit DependencyResolverUnavailable(const std::string &message);
};
