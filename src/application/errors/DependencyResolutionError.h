#pragma once

#include <stdexcept>
#include <string>

class DependencyResolutionError : public std::runtime_error
{
public:
    explicit DependencyResolutionError(const std::string &message);
};
