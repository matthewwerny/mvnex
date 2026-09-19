#pragma once

#include <string>

class ResolvedDependency
{
public:
    ResolvedDependency(
        std::string groupId,
        std::string artifactId,
        std::string version);

    const std::string &groupId() const;
    const std::string &artifactId() const;
    const std::string &version() const;

private:
    std::string groupId_;
    std::string artifactId_;
    std::string version_;
};