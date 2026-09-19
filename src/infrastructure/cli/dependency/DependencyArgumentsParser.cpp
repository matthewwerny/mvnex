#include "infrastructure/cli/dependency/DependencyArgumentsParser.h"

#include <array>
#include <algorithm>
#include <stdexcept>

std::vector<DependencyRequest> DependencyArgumentsParser::parse(
    const std::vector<std::string> &dependencies,
    const std::string &version,
    const std::string &scope) const
{
    if (dependencies.empty())
    {
        throw std::invalid_argument("Missing dependency.");
    }

    if (!version.empty() && dependencies.size() > 1)
    {
        throw std::invalid_argument("--version can only be used when adding a single dependency.");
    }

    if (!scope.empty() && dependencies.size() > 1)
    {
        throw std::invalid_argument("--scope can only be used when adding a single dependency.");
    }

    validateScope(scope);

    std::vector<DependencyRequest> requests;
    requests.reserve(dependencies.size());

    for (const std::string &dependency : dependencies)
    {
        requests.push_back(parseDependency(dependency, version, scope));
    }

    return requests;
}

DependencyRequest DependencyArgumentsParser::parseDependency(
    const std::string &dependency,
    const std::string &optionVersion,
    const std::string &scope) const
{
    std::vector<std::string> parts = splitDependency(dependency);

    if (parts.empty() || parts.size() > 3)
    {
        throw std::invalid_argument("Invalid dependency format: " + dependency);
    }

    for (const std::string &part : parts)
    {
        if (part.empty())
        {
            throw std::invalid_argument("Invalid dependency format: " + dependency);
        }
    }

    if (parts.size() == 1)
    {
        std::string version = resolveVersion("", optionVersion);

        if (version.empty())
        {
            return DependencyRequest::searchTerm(parts[0], scope);
        }

        return DependencyRequest::searchTermWithVersion(parts[0], version, scope);
    }

    if (parts.size() == 2)
    {
        const std::string &first = parts[0];
        const std::string &second = parts[1];

        bool looksLikeCoordinate = first.find('.') != std::string::npos;

        if (looksLikeCoordinate)
        {
            std::string version = resolveVersion("", optionVersion);

            if (version.empty())
            {
                return DependencyRequest::coordinate(first, second, scope);
            }

            return DependencyRequest::coordinateWithVersion(first, second, version, scope);
        }

        std::string version = resolveVersion(second, optionVersion);

        return DependencyRequest::searchTermWithVersion(first, version, scope);
    }

    const std::string &groupId = parts[0];
    const std::string &artifactId = parts[1];
    const std::string &inlineVersion = parts[2];

    std::string version = resolveVersion(inlineVersion, optionVersion);

    return DependencyRequest::coordinateWithVersion(groupId, artifactId, version, scope);
}

std::vector<std::string> DependencyArgumentsParser::splitDependency(
    const std::string &dependency) const
{
    std::vector<std::string> parts;
    std::string current;

    for (char character : dependency)
    {
        if (character == ':')
        {
            parts.push_back(current);
            current.clear();
        }
        else
        {
            current += character;
        }
    }

    parts.push_back(current);

    return parts;
}

std::string DependencyArgumentsParser::resolveVersion(
    const std::string &inlineVersion,
    const std::string &optionVersion) const
{
    if (!inlineVersion.empty() &&
        !optionVersion.empty() &&
        inlineVersion != optionVersion)
    {
        throw std::invalid_argument(
            "Dependency version was provided twice with different values.");
    }

    if (!inlineVersion.empty())
    {
        return inlineVersion;
    }

    return optionVersion;
}

void DependencyArgumentsParser::validateScope(const std::string &scope) const
{
    if (scope.empty())
    {
        return;
    }

    constexpr std::array<const char *, 6> validScopes = {
        "compile",
        "provided",
        "runtime",
        "test",
        "system",
        "import"};

    bool isValidScope = std::any_of(
        validScopes.begin(),
        validScopes.end(),
        [&scope](const char *validScope)
        {
            return scope == validScope;
        });

    if (!isValidScope)
    {
        throw std::invalid_argument("Invalid dependency scope: " + scope);
    }
}
