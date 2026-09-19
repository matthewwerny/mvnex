#pragma once

#include "application/ports/DependencyResolver.h"
#include "application/ports/HttpClient.h"

#include <string>
#include <vector>

class MavenCentralDependencyResolver : public DependencyResolver
{
public:
    explicit MavenCentralDependencyResolver(const HttpClient &httpClient);
    MavenCentralDependencyResolver(
        const HttpClient &httpClient,
        std::string searchUrl);

    ResolvedDependency resolveBySearchTerm(
        const std::string &term,
        const std::string &version) const override;

    ResolvedDependency resolveByCoordinate(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version) const override;

private:
    struct MavenArtifact
    {
        std::string groupId;
        std::string artifactId;
        std::string latestVersion;
    };

    const HttpClient &httpClient_;
    std::string searchUrl_;

    std::vector<MavenArtifact> searchArtifacts(const std::string &query) const;
    bool versionExists(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version) const;

    std::string latestVersion(
        const std::string &groupId,
        const std::string &artifactId) const;

    std::string latestStableVersion(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &fallbackVersion) const;

    std::string buildSearchUrl(
        const std::string &query,
        int rows,
        const std::string &core = "") const;

    static std::string buildSearchUrl(
        const std::string &baseUrl,
        const std::string &query,
        int rows,
        const std::string &core = "");

    static std::string encodeQueryValue(const std::string &value);
    static std::string pomUrl(
        const std::string &groupId,
        const std::string &artifactId,
        const std::string &version);
    static std::string metadataUrl(
        const std::string &groupId,
        const std::string &artifactId);
};
