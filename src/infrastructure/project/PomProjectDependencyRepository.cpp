#include "infrastructure/project/PomProjectDependencyRepository.h"

#include "infrastructure/filesystem/FileSystem.h"

#include <regex>
#include <stdexcept>
#include <sstream>

namespace
{
constexpr const char *PomFileName = "pom.xml";

std::string trim(const std::string &value)
{
    const std::size_t start = value.find_first_not_of(" \t\n\r");

    if (start == std::string::npos)
    {
        return "";
    }

    const std::size_t end = value.find_last_not_of(" \t\n\r");
    return value.substr(start, end - start + 1);
}

std::string xmlTagValue(const std::string &content, const std::string &tag)
{
    std::regex expression("<" + tag + R"(>\s*([^<]+)\s*</)" + tag + ">");
    std::smatch match;

    if (!std::regex_search(content, match, expression))
    {
        return "";
    }

    return trim(match[1].str());
}
}

PomProjectDependencyRepository::PomProjectDependencyRepository(const FileSystem &fileSystem)
    : fileSystem_(fileSystem)
{
}

bool PomProjectDependencyRepository::projectFileExists() const
{
    try
    {
        locatePomFile();
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool PomProjectDependencyRepository::containsDependency(
    const std::string &groupId,
    const std::string &artifactId) const
{
    std::filesystem::path pomPath = locatePomFile();
    std::unordered_set<std::string> existingDependencies =
        loadExistingDependencies(pomPath);

    return existingDependencies.contains(dependencyKey(groupId, artifactId));
}

void PomProjectDependencyRepository::addDependencies(
    const std::vector<ResolvedDependency> &dependencies) const
{
    if (dependencies.empty())
    {
        return;
    }

    std::filesystem::path pomPath = locatePomFile();
    std::string pomContent = fileSystem_.readFile(pomPath);
    fileSystem_.writeFile(
        pomPath,
        insertDependencies(pomContent, dependencies));
}

std::filesystem::path PomProjectDependencyRepository::locatePomFile() const
{
    std::filesystem::path current = std::filesystem::current_path();

    while (true)
    {
        std::filesystem::path candidate = current / PomFileName;

        if (fileSystem_.exists(candidate))
        {
            return candidate;
        }

        if (!current.has_parent_path() || current == current.parent_path())
        {
            break;
        }

        current = current.parent_path();
    }

    throw std::runtime_error("pom.xml not found. Run this command inside a Maven project.");
}

std::unordered_set<std::string> PomProjectDependencyRepository::loadExistingDependencies(
    const std::filesystem::path &pomPath) const
{
    std::string pomContent = fileSystem_.readFile(pomPath);
    std::unordered_set<std::string> dependencies;
    std::regex dependencyExpression(R"(<dependency\b[^>]*>([\s\S]*?)</dependency>)");

    auto begin = std::sregex_iterator(
        pomContent.begin(),
        pomContent.end(),
        dependencyExpression);
    auto end = std::sregex_iterator();

    for (auto iterator = begin; iterator != end; ++iterator)
    {
        std::string dependencyBlock = (*iterator)[1].str();
        std::string groupId = xmlTagValue(dependencyBlock, "groupId");
        std::string artifactId = xmlTagValue(dependencyBlock, "artifactId");

        if (!groupId.empty() && !artifactId.empty())
        {
            dependencies.insert(dependencyKey(groupId, artifactId));
        }
    }

    return dependencies;
}

std::string PomProjectDependencyRepository::dependencyKey(
    const std::string &groupId,
    const std::string &artifactId)
{
    return groupId + ":" + artifactId;
}

std::string PomProjectDependencyRepository::dependencyXml(const ResolvedDependency &dependency)
{
    std::ostringstream xml;
    xml
        << "        <dependency>\n"
        << "            <groupId>" << dependency.groupId() << "</groupId>\n"
        << "            <artifactId>" << dependency.artifactId() << "</artifactId>\n"
        << "            <version>" << dependency.version() << "</version>\n"
        << "        </dependency>\n";

    return xml.str();
}

std::string PomProjectDependencyRepository::insertDependencies(
    const std::string &pomContent,
    const std::vector<ResolvedDependency> &dependencies)
{
    std::ostringstream dependenciesXml;

    for (const ResolvedDependency &dependency : dependencies)
    {
        dependenciesXml << dependencyXml(dependency);
    }

    const std::string dependenciesCloseTag = "    </dependencies>";
    std::size_t dependenciesClosePosition = pomContent.rfind(dependenciesCloseTag);

    if (dependenciesClosePosition != std::string::npos)
    {
        std::string content = pomContent;
        content.insert(dependenciesClosePosition, dependenciesXml.str());
        return content;
    }

    const std::string projectCloseTag = "</project>";
    std::size_t projectClosePosition = pomContent.rfind(projectCloseTag);

    if (projectClosePosition == std::string::npos)
    {
        throw std::runtime_error("Invalid pom.xml: missing </project>.");
    }

    std::ostringstream dependenciesBlock;
    dependenciesBlock
        << "    <dependencies>\n"
        << dependenciesXml.str()
        << "    </dependencies>\n\n";

    std::string content = pomContent;
    content.insert(projectClosePosition, dependenciesBlock.str());

    return content;
}
