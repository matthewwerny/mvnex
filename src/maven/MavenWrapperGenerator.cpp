#include "MavenWrapperGenerator.h"
#include "MavenWrapperFiles.h"
#include "cli/Style.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace fs = std::filesystem;

static void writeFile(const fs::path &path, const std::string &content)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error("Could not create " + path.string());
    }

    file << content;

    if (!file)
    {
        throw std::runtime_error("Could not write " + path.string());
    }
}

void MavenWrapperGenerator::generate(const std::filesystem::path &projectPath)
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
) {
    std::filesystem::path pomPath = projectPath / "pom.xml";

    std::string command =
        "mvn -q -f \"" + pomPath.string() + "\" "
        "org.apache.maven.plugins:maven-wrapper-plugin:3.3.4:wrapper";

#ifdef _WIN32
    command += " > NUL 2>&1";
#else
    command += " > /dev/null 2>&1";
#endif

    auto task = std::async(
        std::launch::async,
        [command]()
        {
            return std::system(command.c_str());
        }
    );

    const char frames[] = {'|', '/', '-', '\\'};
    std::size_t frame = 0;

    while (task.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
    {
        std::cout
            << "\r  "
            << Style::CYAN
            << frames[frame++ % 4]
            << Style::RESET
            << " Generating Maven Wrapper..."
            << std::flush;
    }

    int result = task.get();

    if (result != 0) {
        std::cout
            << "\r  "
            << Style::CYAN
            << "✗"
            << Style::RESET
            << " Maven Wrapper generation failed"
            << "     "
            << '\n';

        throw std::runtime_error("Failed to generate Maven Wrapper.");
    }

    std::cout
        << "\r  "
        << Style::CYAN
        << "✓"
        << Style::RESET
        << " Maven Wrapper generated     "
        << '\n';
}

void MavenWrapperGenerator::generateStandalone(
    const std::filesystem::path& projectPath
) {
    fs::path wrapperDir = projectPath / ".mvn" / "wrapper";

    fs::create_directories(wrapperDir);

    writeFile(wrapperDir / "maven-wrapper.properties", MavenWrapperFiles::properties());
    writeFile(projectPath / "mvnw", MavenWrapperFiles::unixScript());
    writeFile(projectPath / "mvnw.cmd", MavenWrapperFiles::windowsScript());

#ifndef _WIN32
    fs::permissions(
        projectPath / "mvnw",
        fs::perms::owner_exec |
            fs::perms::group_exec |
            fs::perms::others_exec,
        fs::perm_options::add
    );
#endif
}
