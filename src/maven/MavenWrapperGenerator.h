#pragma once
#include <filesystem>

class MavenWrapperGenerator {
public:
    static void generate(const std::filesystem::path& projectPath);
    static bool isMavenInstalled();

private:
    static void generateUsingMaven(const std::filesystem::path& projectPath);
    static void generateStandalone(const std::filesystem::path& projectPath);
};
