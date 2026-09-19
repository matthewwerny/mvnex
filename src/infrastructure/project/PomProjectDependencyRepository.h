#pragma once

#include "application/ports/ProjectDependencyRepository.h"

#include <filesystem>
#include <string>
#include <unordered_set>

class FileSystem;

class PomProjectDependencyRepository : public ProjectDependencyRepository
{
public:
    explicit PomProjectDependencyRepository(const FileSystem &fileSystem);

    bool projectFileExists() const override;

    bool containsDependency(
        const std::string &groupId,
        const std::string &artifactId) const override;

    void addDependencies(
        const std::vector<ResolvedDependency> &dependencies) const override;

private:
    const FileSystem &fileSystem_;

    std::filesystem::path locatePomFile() const;
    std::unordered_set<std::string> loadExistingDependencies(
        const std::filesystem::path &pomPath) const;

    static std::string dependencyKey(
        const std::string &groupId,
        const std::string &artifactId);
    static std::string dependencyXml(const ResolvedDependency &dependency);
    static std::string insertDependencies(
        const std::string &pomContent,
        const std::vector<ResolvedDependency> &dependencies);
};
