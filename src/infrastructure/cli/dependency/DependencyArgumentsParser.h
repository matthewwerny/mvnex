#pragma once

#include "domain/dependency/DependencyRequest.h"

#include <string>
#include <vector>

class DependencyArgumentsParser
{
public:
    std::vector<DependencyRequest> parse(
        const std::vector<std::string> &dependencies,
        const std::string &version
    ) const;

private:
    DependencyRequest parseDependency(
        const std::string &dependency,
        const std::string &version
    ) const;

    std::vector<std::string> splitDependency(
        const std::string &dependency
    ) const;

    std::string resolveVersion(
        const std::string &inlineVersion,
        const std::string &optionVersion
    ) const;
};