#include "MavenWrapperGenerator.h"
#include "MavenWrapperFiles.h"
#include "application/ports/ProgressReporter.h"
#include "infrastructure/filesystem/FileSystem.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

MavenWrapperGenerator::MavenWrapperGenerator(
    const FileSystem &fileSystem,
    const ProgressReporter &progressReporter)
    : fileSystem_(fileSystem),
      progressReporter_(progressReporter)
{
}

void MavenWrapperGenerator::generate(const std::filesystem::path &projectPath) const
{
    if (isMavenInstalled())
    {
        generateUsingMaven(projectPath);
    }
    else
    {
        generateStandalone(projectPath);
    }
}

bool MavenWrapperGenerator::isMavenInstalled()
{
    #ifdef _WIN32
        int result = std::system("mvn --version > NUL 2>&1");   
    #else
        int result = std::system("mvn --version > /dev/null 2>&1");
    #endif

    return result == 0;
}

void MavenWrapperGenerator::generateUsingMaven(
    const std::filesystem::path& projectPath
) const {
    std::filesystem::path pomPath = projectPath / "pom.xml";

    std::string command =
        "mvn -q -f \"" + pomPath.string() + "\" "
        "org.apache.maven.plugins:maven-wrapper-plugin:3.3.4:wrapper";

#ifdef _WIN32
    command += " > NUL 2>&1";
#else
    command += " > /dev/null 2>&1";
#endif

    progressReporter_.run(
        "Generating Maven Wrapper...",
        [&command]()
        {
            int result = std::system(command.c_str());

            if (result != 0) {
                throw std::runtime_error("Failed to generate Maven Wrapper.");
            }
        });
}

void MavenWrapperGenerator::generateStandalone(
    const std::filesystem::path& projectPath
) const {
    fs::path wrapperDir = projectPath / ".mvn" / "wrapper";

    fileSystem_.createDirectories(wrapperDir);

    fileSystem_.writeFile(wrapperDir / "maven-wrapper.properties", MavenWrapperFiles::properties());
    fileSystem_.writeFile(projectPath / "mvnw", MavenWrapperFiles::unixScript());
    fileSystem_.writeFile(projectPath / "mvnw.cmd", MavenWrapperFiles::windowsScript());

    fileSystem_.addOwnerGroupOthersExecute(projectPath / "mvnw");
}
