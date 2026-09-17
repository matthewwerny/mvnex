#pragma once
#include <filesystem>

class FileSystem;
class ProgressReporter;

class MavenWrapperGenerator {
public:
    MavenWrapperGenerator(
        const FileSystem &fileSystem,
        const ProgressReporter &progressReporter);

    void generate(const std::filesystem::path& projectPath) const;
    static bool isMavenInstalled();

private:
    void generateUsingMaven(const std::filesystem::path& projectPath) const;
    void generateStandalone(const std::filesystem::path& projectPath) const;

    const FileSystem &fileSystem_;
    const ProgressReporter &progressReporter_;
};
