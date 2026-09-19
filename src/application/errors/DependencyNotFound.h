#pragma once

#include "application/errors/DependencyResolutionError.h"

#include <string>

class DependencyNotFound : public DependencyResolutionError
{
public:
    explicit DependencyNotFound(const std::string &query);

    const std::string &query() const;

private:
    std::string query_;
};
