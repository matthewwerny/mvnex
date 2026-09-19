#include "infrastructure/dependency/DepsDevDependencyResolver.h"

#include "application/errors/DependencyNotFound.h"
#include "application/errors/DependencyResolutionError.h"

#include <cctype>
#include <regex>
#include <sstream>

namespace
{
constexpr const char *DepsDevBaseUrl = "https://api.deps.dev/v3/systems/MAVEN/packages/";

std::vector<std::string> jsonVersionObjects(const std::string &body)
{
    std::vector<std::string> versions;
    std::size_t versionsPosition = body.find("\"versions\"");

    if (versionsPosition == std::string::npos)
    {
        return versions;
    }

    std::size_t arrayStart = body.find('[', versionsPosition);
    if (arrayStart == std::string::npos)
    {
        return versions;
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
                versions.push_back(body.substr(objectStart, i - objectStart + 1));
                objectStart = std::string::npos;
            }
        }
        else if (body[i] == ']' && depth == 0)
        {
            break;
        }
    }

    return versions;
}

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

bool jsonBoolValue(const std::string &document, const std::string &field)
{
    std::regex expression("\"" + field + "\"\\s*:\\s*(true|false)");
    std::smatch match;

    if (!std::regex_search(document, match, expression))
    {
        return false;
    }

    return match[1].str() == "true";
}
}

DepsDevDependencyResolver::DepsDevDependencyResolver(const HttpClient &httpClient)
    : httpClient_(httpClient)
{
}

ResolvedDependency DepsDevDependencyResolver::resolveBySearchTerm(
    const std::string &term,
    const std::string &) const
{
    throw DependencyNotFound(term);
}

ResolvedDependency DepsDevDependencyResolver::resolveByCoordinate(
    const std::string &groupId,
    const std::string &artifactId,
    const std::string &version) const
{
    std::string coordinate = groupId + ":" + artifactId;
    std::vector<PackageVersion> versions = fetchVersions(groupId, artifactId);

    if (versions.empty())
    {
        throw DependencyNotFound(coordinate);
    }

    if (!version.empty())
    {
        if (!hasVersion(versions, version))
        {
            throw DependencyNotFound(coordinate + ":" + version);
        }

        return ResolvedDependency(groupId, artifactId, version);
    }

    return ResolvedDependency(
        groupId,
        artifactId,
        defaultVersion(versions, coordinate));
}

std::vector<DepsDevDependencyResolver::PackageVersion>
DepsDevDependencyResolver::fetchVersions(
    const std::string &groupId,
    const std::string &artifactId) const
{
    HttpResponse response = httpClient_.get(packageUrl(groupId, artifactId));

    if (response.statusCode == 404)
    {
        return {};
    }

    if (response.statusCode < 200 || response.statusCode >= 300)
    {
        throw DependencyResolutionError("deps.dev request failed.");
    }

    std::vector<PackageVersion> versions;

    for (const std::string &versionObject : jsonVersionObjects(response.body))
    {
        std::string version = jsonStringValue(versionObject, "version");

        if (version.empty())
        {
            continue;
        }

        versions.push_back(PackageVersion{
            version,
            jsonBoolValue(versionObject, "isDefault")});
    }

    return versions;
}

bool DepsDevDependencyResolver::hasVersion(
    const std::vector<PackageVersion> &versions,
    const std::string &version) const
{
    for (const PackageVersion &candidate : versions)
    {
        if (candidate.value == version)
        {
            return true;
        }
    }

    return false;
}

std::string DepsDevDependencyResolver::defaultVersion(
    const std::vector<PackageVersion> &versions,
    const std::string &coordinate) const
{
    for (const PackageVersion &version : versions)
    {
        if (version.isDefault)
        {
            return version.value;
        }
    }

    if (!versions.empty())
    {
        return versions.front().value;
    }

    throw DependencyNotFound(coordinate);
}

std::string DepsDevDependencyResolver::packageUrl(
    const std::string &groupId,
    const std::string &artifactId)
{
    return std::string(DepsDevBaseUrl) +
           encodePathSegment(groupId + ":" + artifactId);
}

std::string DepsDevDependencyResolver::encodePathSegment(const std::string &value)
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

        encoded << '%';
        encoded << "0123456789ABCDEF"[character >> 4];
        encoded << "0123456789ABCDEF"[character & 15];
    }

    return encoded.str();
}
