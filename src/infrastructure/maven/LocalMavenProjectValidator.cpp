#include "infrastructure/maven/LocalMavenProjectValidator.h"

#include "infrastructure/filesystem/FileSystem.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace
{
std::string quote(const std::filesystem::path &path)
{
    return "\"" + path.string() + "\"";
}
}

LocalMavenProjectValidator::LocalMavenProjectValidator(const FileSystem &fileSystem)
    : fileSystem_(fileSystem)
{
}

MavenProjectValidationResult LocalMavenProjectValidator::validate() const
{
    std::filesystem::path projectRoot = locateProjectRoot();

    if (hasMavenWrapper(projectRoot))
    {
        return {
            runValidate(projectRoot, true)
                ? MavenProjectValidationStatus::Passed
                : MavenProjectValidationStatus::Failed};
    }

    if (!hasLocalMaven())
    {
        return {MavenProjectValidationStatus::MavenNotFound};
    }

    return {
        runValidate(projectRoot, false)
            ? MavenProjectValidationStatus::Passed
            : MavenProjectValidationStatus::Failed};
}

std::filesystem::path LocalMavenProjectValidator::locateProjectRoot() const
{
    std::filesystem::path current = std::filesystem::current_path();

    while (true)
    {
        if (fileSystem_.exists(current / "pom.xml"))
        {
            return current;
        }

        if (!current.has_parent_path() || current == current.parent_path())
        {
            throw std::runtime_error("pom.xml not found. Run this command inside a Maven project.");
        }

        current = current.parent_path();
    }
}

bool LocalMavenProjectValidator::hasMavenWrapper(const std::filesystem::path &projectRoot) const
{
#ifdef _WIN32
    return fileSystem_.exists(projectRoot / "mvnw.cmd");
#else
    return fileSystem_.exists(projectRoot / "mvnw");
#endif
}

bool LocalMavenProjectValidator::hasLocalMaven() const
{
#ifdef _WIN32
    int result = std::system("mvn --version > NUL 2>&1");
#else
    int result = std::system("mvn --version > /dev/null 2>&1");
#endif

    return result == 0;
}

bool LocalMavenProjectValidator::runValidate(
    const std::filesystem::path &projectRoot,
    bool useWrapper) const
{
#ifdef _WIN32
    std::string command = useWrapper
        ? "cd /d " + quote(projectRoot) + " && mvnw.cmd -q validate > NUL 2>&1"
        : "mvn -q -f " + quote(projectRoot / "pom.xml") + " validate > NUL 2>&1";
#else
    std::string command = useWrapper
        ? "cd " + quote(projectRoot) + " && ./mvnw -q validate > /dev/null 2>&1"
        : "mvn -q -f " + quote(projectRoot / "pom.xml") + " validate > /dev/null 2>&1";
#endif

    return std::system(command.c_str()) == 0;
}
