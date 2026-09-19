#pragma once

#include "application/ports/DependencyResolver.h"
#include "application/ports/HttpClient.h"

#include <string>
#include <vector>

class DepsDevDependencyResolver : public DependencyResolver
{
public:
    explicit DepsDevDependencyResolver(const HttpClient &httpClient);

    ResolvedDependency resolveBySearchTerm(
        const std::string &term,
        const std::string &version) const override;

    ResolvedDependency resolveByCoordinate(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version) const override;

private:
    struct PackageVersion
    {
        std::string value;
        bool isDefault;
    };

    const HttpClient &httpClient_;

    std::vector<PackageVersion> fetchVersions(
        const std::string &groupId,
        const std::string &artifactId) const;

    bool hasVersion(
        const std::vector<PackageVersion> &versions,
        const std::string &version) const;

    std::string defaultVersion(
        const std::vector<PackageVersion> &versions,
        const std::string &coordinate) const;

    static std::string packageUrl(
        const std::string &groupId,
        const std::string &artifactId);

    static std::string encodePathSegment(const std::string &value);
};
