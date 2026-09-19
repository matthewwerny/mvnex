#pragma once

#include <string>

class ResolvedDependency
{
public:
    ResolvedDependency(
        std::string groupId,
        std::string artifactId,
        std::string version,
        std::string scope = "");

    const std::string &groupId() const;
    const std::string &artifactId() const;
    const std::string &version() const;
    const std::string &scope() const;

private:
    std::string groupId_;
    std::string artifactId_;
    std::string version_;
    std::string scope_;
};
