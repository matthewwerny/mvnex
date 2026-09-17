#include "infrastructure/project/ProjectGenerator.h"

#include "infrastructure/filesystem/FileSystem.h"
#include "infrastructure/maven/MavenWrapperGenerator.h"

#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

static fs::path packageToPath(const std::string &packageName)
{
    fs::path path;
    std::string current;

    for (char c : packageName)
    {
        if (c == '.')
        {
            path /= current;
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty())
    {
        path /= current;
    }

    return path;
}

ProjectGenerator::ProjectGenerator(
    const FileSystem &fileSystem,
    MavenWrapperGenerator &mavenWrapperGenerator)
    : fileSystem_(fileSystem),
      mavenWrapperGenerator_(mavenWrapperGenerator)
{
}

void ProjectGenerator::create(const ProjectConfig &config, bool skipWrapper) const
{
    fs::path projectPath = config.name;

    if (fileSystem_.exists(projectPath))
    {
        throw std::invalid_argument(
            "Project directory already exists: " + config.name
        );
    }

    fs::path packagePath = packageToPath(config.packageName);

    fileSystem_.createDirectories(
        projectPath /
        "src" /
        "main" /
        "java" /
        packagePath
    );

    fileSystem_.createDirectories(
        projectPath /
        "src" /
        "test" /
        "java" /
        packagePath
    );

    fs::path mainJavaPath =
        projectPath /
        "src" /
        "main" /
        "java" /
        packagePath /
        "Main.java";

    fileSystem_.writeFile(
        mainJavaPath,
        "package " + config.packageName + ";\n\n"
        "public class Main {\n"
        "    public static void main(String[] args) {\n"
        "        System.out.println(\"Hello from " + config.name + "!\");\n"
        "    }\n"
        "}\n");

    fs::path pomPath = projectPath / "pom.xml";

    fileSystem_.writeFile(
        pomPath,
        "<project xmlns=\"http://maven.apache.org/POM/4.0.0\"\n"
        "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
        "         xsi:schemaLocation=\"http://maven.apache.org/POM/4.0.0 "
        "https://maven.apache.org/xsd/maven-4.0.0.xsd\">\n"
        "    <modelVersion>4.0.0</modelVersion>\n\n"
        "    <groupId>" + config.groupId + "</groupId>\n"
        "    <artifactId>" + config.name + "</artifactId>\n"
        "    <version>1.0-SNAPSHOT</version>\n\n"
        "    <properties>\n"
        "        <maven.compiler.release>" + config.javaVersion + "</maven.compiler.release>\n"
        "        <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding>\n"
        "    </properties>\n"
        "</project>\n");

    if (!skipWrapper)
    {
        mavenWrapperGenerator_.generate(projectPath);
    }
}
