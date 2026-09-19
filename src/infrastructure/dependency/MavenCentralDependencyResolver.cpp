#include "infrastructure/dependency/MavenCentralDependencyResolver.h"

#include "application/errors/DependencyNotFound.h"
#include "application/errors/DependencyResolutionError.h"
#include "application/errors/MultipleDependencyMatches.h"

#include <cctype>
#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr const char *MavenCentralSearchUrl = "https://search.maven.org/solrsearch/select";
constexpr const char *MavenCentralRepositoryUrl = "https://repo1.maven.org/maven2";

std::string jsonStringValue(const std::string &document, const std::string &field)
{
    std::regex expression("\"" + field + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;

    if (!std::regex_search(document, match, expression))
    {
        return "";
    }

    return match[1].str();
}

int jsonIntValue(const std::string &document, const std::string &field)
{
    std::regex expression("\"" + field + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;

    if (!std::regex_search(document, match, expression))
    {
        return 0;
    }

    return std::stoi(match[1].str());
}

std::vector<std::string> jsonDocs(const std::string &body)
{
    std::vector<std::string> docs;
    std::size_t docsPosition = body.find("\"docs\"");

    if (docsPosition == std::string::npos)
    {
        return docs;
    }

    std::size_t arrayStart = body.find('[', docsPosition);
    if (arrayStart == std::string::npos)
    {
        return docs;
    }

    int depth = 0;
    std::size_t objectStart = std::string::npos;

    for (std::size_t i = arrayStart + 1; i < body.size(); ++i)
    {
        if (body[i] == '{')
        {
            if (depth == 0)
            {
                objectStart = i;
            }

            ++depth;
        }
        else if (body[i] == '}')
        {
            --depth;

            if (depth == 0 && objectStart != std::string::npos)
            {
                docs.push_back(body.substr(objectStart, i - objectStart + 1));
                objectStart = std::string::npos;
            }
        }
        else if (body[i] == ']' && depth == 0)
        {
            break;
        }
    }

    return docs;
}

bool containsPreReleaseMarker(const std::string &version)
{
    std::string lowerVersion = version;

    std::transform(
        lowerVersion.begin(),
        lowerVersion.end(),
        lowerVersion.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        });

    return lowerVersion.find("-m") != std::string::npos ||
           lowerVersion.find("-rc") != std::string::npos ||
           lowerVersion.find("-alpha") != std::string::npos ||
           lowerVersion.find("-beta") != std::string::npos ||
           lowerVersion.find("-snapshot") != std::string::npos;
}

std::vector<std::string> xmlTagValues(
    const std::string &body,
    const std::string &tag)
{
    std::vector<std::string> values;
    std::string openingTag = "<" + tag + ">";
    std::string closingTag = "</" + tag + ">";
    std::size_t position = 0;

    while (true)
    {
        std::size_t start = body.find(openingTag, position);

        if (start == std::string::npos)
        {
            break;
        }

        start += openingTag.size();

        std::size_t end = body.find(closingTag, start);

        if (end == std::string::npos)
        {
            break;
        }

        values.push_back(body.substr(start, end - start));
        position = end + closingTag.size();
    }

    return values;
}

int knownGroupRank(const std::string &term, const std::string &groupId)
{
    const std::vector<std::pair<std::string, std::string>> knownGroups{
        {"lombok", "org.projectlombok"},
        {"spring-", "org.springframework"},
        {"spring-boot-", "org.springframework.boot"},
        {"junit", "org.junit.jupiter"},
        {"slf4j-", "org.slf4j"},
        {"logback-", "ch.qos.logback"},
        {"jackson-", "com.fasterxml.jackson"},
        {"postgresql", "org.postgresql"},
        {"mysql", "com.mysql"},
        {"guava", "com.google.guava"}};

    for (std::size_t i = 0; i < knownGroups.size(); ++i)
    {
        const auto &[termPrefix, groupPrefix] = knownGroups[i];

        if (term.rfind(termPrefix, 0) == 0 &&
            groupId.rfind(groupPrefix, 0) == 0)
        {
            return static_cast<int>(i);
        }
    }

    return 1000;
}

int suspiciousGroupPenalty(const std::string &groupId)
{
    if (groupId.rfind("io.github.", 0) == 0 ||
        groupId.rfind("com.github.", 0) == 0)
    {
        return 100;
    }

    return 0;
}

int candidateScore(const std::string &term, const ResolvedDependency &dependency)
{
    int score = 0;
    score += knownGroupRank(term, dependency.groupId()) * 10;
    score += suspiciousGroupPenalty(dependency.groupId());

    if (containsPreReleaseMarker(dependency.version()))
    {
        score += 50;
    }

    return score;
}
}

MavenCentralDependencyResolver::MavenCentralDependencyResolver(const HttpClient &httpClient)
    : MavenCentralDependencyResolver(httpClient, MavenCentralSearchUrl)
{
}

MavenCentralDependencyResolver::MavenCentralDependencyResolver(
    const HttpClient &httpClient,
    std::string searchUrl)
    : httpClient_(httpClient),
      searchUrl_(std::move(searchUrl))
{
}

ResolvedDependency MavenCentralDependencyResolver::resolveBySearchTerm(
    const std::string &term,
    const std::string &version) const
{
    std::vector<MavenArtifact> artifacts = searchArtifacts("a:" + term);

    if (artifacts.empty())
    {
        artifacts = searchArtifacts(term);
    }

    std::vector<ResolvedDependency> candidates;

    for (const MavenArtifact &artifact : artifacts)
    {
        if (artifact.artifactId != term)
        {
            continue;
        }

        std::string resolvedVersion = version.empty()
                                          ? latestStableVersion(
                                                artifact.groupId,
                                                artifact.artifactId,
                                                artifact.latestVersion)
                                          : version;

        candidates.emplace_back(
            artifact.groupId,
            artifact.artifactId,
            resolvedVersion);
    }

    if (candidates.empty())
    {
        throw DependencyNotFound(term);
    }

    std::stable_sort(
        candidates.begin(),
        candidates.end(),
        [&term](const ResolvedDependency &left, const ResolvedDependency &right)
        {
            return candidateScore(term, left) < candidateScore(term, right);
        });

    if (candidates.size() > 1)
    {
        throw MultipleDependencyMatches(term, candidates);
    }

    const ResolvedDependency &resolved = candidates[0];

    if (!version.empty() &&
        !versionExists(resolved.groupId(), resolved.artifactId(), version))
    {
        throw DependencyNotFound(
            resolved.groupId() + ":" + resolved.artifactId() + ":" + version);
    }

    return resolved;
}

ResolvedDependency MavenCentralDependencyResolver::resolveByCoordinate(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &version) const
{
    if (!version.empty())
    {
        if (!versionExists(groupId, artifactId, version))
        {
            throw DependencyNotFound(groupId + ":" + artifactId + ":" + version);
        }

        return ResolvedDependency(groupId, artifactId, version);
    }

    return ResolvedDependency(
        groupId,
        artifactId,
        latestVersion(groupId, artifactId));
}

std::vector<MavenCentralDependencyResolver::MavenArtifact>
MavenCentralDependencyResolver::searchArtifacts(const std::string &query) const
{
    HttpResponse response = httpClient_.get(buildSearchUrl(query, 25));

    if (response.statusCode < 200 || response.statusCode >= 300)
    {
        throw DependencyResolutionError("Maven Central request failed.");
    }

    std::vector<MavenArtifact> artifacts;

    for (const std::string &doc : jsonDocs(response.body))
    {
        MavenArtifact artifact{
            jsonStringValue(doc, "g"),
            jsonStringValue(doc, "a"),
            jsonStringValue(doc, "latestVersion")};

        if (!artifact.groupId.empty() &&
            !artifact.artifactId.empty() &&
            !artifact.latestVersion.empty())
        {
            artifacts.push_back(artifact);
        }
    }

    return artifacts;
}

bool MavenCentralDependencyResolver::versionExists(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &version) const
{
    std::string query =
        "g:\"" + groupId + "\" AND a:\"" + artifactId + "\" AND v:\"" + version + "\"";

    HttpResponse response = httpClient_.get(buildSearchUrl(query, 1, "gav"));

    if (response.statusCode < 200 || response.statusCode >= 300)
    {
        throw DependencyResolutionError("Maven Central request failed.");
    }

    if (jsonIntValue(response.body, "numFound") > 0)
    {
        return true;
    }

    HttpResponse pomResponse = httpClient_.get(pomUrl(groupId, artifactId, version));

    if (pomResponse.statusCode == 404)
    {
        return false;
    }

    if (pomResponse.statusCode < 200 || pomResponse.statusCode >= 300)
    {
        throw DependencyResolutionError("Maven Central artifact request failed.");
    }

    return true;
}

std::string MavenCentralDependencyResolver::latestVersion(
    const std::string &groupId,
    const std::string &artifactId) const
{
    std::string query = "g:\"" + groupId + "\" AND a:\"" + artifactId + "\"";
    std::vector<MavenArtifact> artifacts = searchArtifacts(query);

    for (const MavenArtifact &artifact : artifacts)
    {
        if (artifact.groupId == groupId && artifact.artifactId == artifactId)
        {
            return artifact.latestVersion;
        }
    }

    throw DependencyNotFound(groupId + ":" + artifactId);
}

std::string MavenCentralDependencyResolver::latestStableVersion(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &fallbackVersion) const
{
    if (!containsPreReleaseMarker(fallbackVersion))
    {
        return fallbackVersion;
    }

    HttpResponse metadataResponse{0, ""};

    try
    {
        metadataResponse = httpClient_.get(metadataUrl(groupId, artifactId));
    }
    catch (const std::exception &)
    {
        return fallbackVersion;
    }

    if (metadataResponse.statusCode >= 200 && metadataResponse.statusCode < 300)
    {
        std::vector<std::string> versions = xmlTagValues(metadataResponse.body, "version");

        for (auto it = versions.rbegin(); it != versions.rend(); ++it)
        {
            if (!containsPreReleaseMarker(*it))
            {
                return *it;
            }
        }
    }

    std::string query = "g:\"" + groupId + "\" AND a:\"" + artifactId + "\"";
    HttpResponse response{0, ""};

    try
    {
        response = httpClient_.get(
            buildSearchUrl(MavenCentralSearchUrl, query, 50, "gav"));
    }
    catch (const std::exception &)
    {
        return fallbackVersion;
    }

    if (response.statusCode < 200 || response.statusCode >= 300)
    {
        return fallbackVersion;
    }

    for (const std::string &doc : jsonDocs(response.body))
    {
        std::string version = jsonStringValue(doc, "v");

        if (!version.empty() && !containsPreReleaseMarker(version))
        {
            return version;
        }
    }

    return fallbackVersion;
}

std::string MavenCentralDependencyResolver::buildSearchUrl(
    const std::string &query,
    int rows,
    const std::string &core) const
{
    return buildSearchUrl(searchUrl_, query, rows, core);
}

std::string MavenCentralDependencyResolver::buildSearchUrl(
    const std::string &baseUrl,
    const std::string &query,
    int rows,
    const std::string &core)
{
    std::ostringstream url;
    url << baseUrl
        << "?q=" << encodeQueryValue(query)
        << "&rows=" << rows
        << "&wt=json";

    if (!core.empty())
    {
        url << "&core=" << encodeQueryValue(core);
    }

    return url.str();
}

std::string MavenCentralDependencyResolver::pomUrl(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &version)
{
    std::string groupPath = groupId;

    for (char &character : groupPath)
    {
        if (character == '.')
        {
            character = '/';
        }
    }

    return std::string(MavenCentralRepositoryUrl) + "/" +
           groupPath + "/" +
           artifactId + "/" +
           version + "/" +
           artifactId + "-" +
           version + ".pom";
}

std::string MavenCentralDependencyResolver::metadataUrl(
    const std::string &groupId,
    const std::string &artifactId)
{
    std::string groupPath = groupId;

    for (char &character : groupPath)
    {
        if (character == '.')
        {
            character = '/';
        }
    }

    return std::string(MavenCentralRepositoryUrl) + "/" +
           groupPath + "/" +
           artifactId + "/maven-metadata.xml";
}

std::string MavenCentralDependencyResolver::encodeQueryValue(const std::string &value)
{
    std::ostringstream encoded;

    for (unsigned char character : value)
    {
        if (std::isalnum(character) ||
            character == '-' ||
            character == '_' ||
            character == '.' ||
            character == '~')
        {
            encoded << character;
            continue;
        }

        if (character == ' ')
        {
            encoded << '+';
            continue;
        }

        encoded << '%';
        encoded << "0123456789ABCDEF"[character >> 4];
        encoded << "0123456789ABCDEF"[character & 15];
    }

    return encoded.str();
}
