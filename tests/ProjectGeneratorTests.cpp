#include "project/ProjectGenerator.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace
{
int failures = 0;

void expect(bool condition, const std::string &message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        ++failures;
    }
}

std::string readFile(const fs::path &path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error("Could not read " + path.string());
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}
}

int main()
{
    fs::path originalPath = fs::current_path();
    auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    fs::path testPath = fs::temp_directory_path() / ("mvnex-project-generator-" + std::to_string(timestamp));

    try
    {
        fs::create_directories(testPath);
        fs::current_path(testPath);

        ProjectGenerator::generate({"demo-app", "com.example", "com.example.demoapp", "21"}, true);

        fs::path projectPath = testPath / "demo-app";
        fs::path mainPath = projectPath / "src/main/java/com/example/demoapp/Main.java";
        fs::path testPackagePath = projectPath / "src/test/java/com/example/demoapp";
        fs::path pomPath = projectPath / "pom.xml";

        expect(fs::exists(mainPath), "Expected Main.java to be generated");
        expect(fs::exists(testPackagePath), "Expected test package directory to be generated");
        expect(fs::exists(pomPath), "Expected pom.xml to be generated");
        expect(!fs::exists(projectPath / "mvnw"), "Expected mvnw not to be generated when wrapper is skipped");
        expect(!fs::exists(projectPath / "mvnw.cmd"), "Expected mvnw.cmd not to be generated when wrapper is skipped");
        expect(!fs::exists(projectPath / ".mvn"), "Expected .mvn not to be generated when wrapper is skipped");

        std::string mainJava = readFile(mainPath);
        std::string pom = readFile(pomPath);

        expect(mainJava.find("package com.example.demoapp;") != std::string::npos,
               "Expected Main.java package to match groupId and project name");
        expect(mainJava.find("Hello from demo-app!") != std::string::npos,
               "Expected Main.java greeting to include project name");
        expect(pom.find("<groupId>com.example</groupId>") != std::string::npos,
               "Expected pom.xml groupId");
        expect(pom.find("<artifactId>demo-app</artifactId>") != std::string::npos,
               "Expected pom.xml artifactId");
        expect(pom.find("<maven.compiler.release>21</maven.compiler.release>") != std::string::npos,
               "Expected pom.xml Java version");
    }
    catch (const std::exception &error)
    {
        std::cerr << "Unexpected exception: " << error.what() << '\n';
        ++failures;
    }

    fs::current_path(originalPath);
    fs::remove_all(testPath);

    if (failures != 0)
    {
        std::cerr << failures << " project generator expectation(s) failed\n";
        return 1;
    }

    return 0;
}
