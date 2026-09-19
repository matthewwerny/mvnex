#pragma once

#include <string>

enum class DependencyRequestType
{
    SearchTerm,
    SearchTermWithVersion,
    Coordinate,
    CoordinateWithVersion
};

class DependencyRequest
{
public:
    static DependencyRequest searchTerm(
        const std::string &query,
        const std::string &scope = "");
    static DependencyRequest searchTermWithVersion(
        const std::string &query,
        const std::string &version,
        const std::string &scope = "");
    static DependencyRequest coordinate(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &scope = "");
    static DependencyRequest coordinateWithVersion(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version,
        const std::string &scope = "");

    DependencyRequestType type() const;

    const std::string &query() const;
    const std::string &groupId() const;
    const std::string &artifactId() const;
    const std::string &version() const;
    const std::string &scope() const;

private:
    DependencyRequest(
        DependencyRequestType type,
        std::string query,
        std::string groupId,
        std::string artifactId,
        std::string version,
        std::string scope);

    DependencyRequestType type_;
    std::string query_;
    std::string groupId_;
    std::string artifactId_;
    std::string version_;
    std::string scope_;
};
