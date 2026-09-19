#pragma once

#include "application/ports/MavenProjectValidator.h"

#include <filesystem>

class FileSystem;

class LocalMavenProjectValidator : public MavenProjectValidator
{
public:
    explicit LocalMavenProjectValidator(const FileSystem &fileSystem);

    MavenProjectValidationResult validate() const override;

private:
    const FileSystem &fileSystem_;

    std::filesystem::path locateProjectRoot() const;
    bool hasMavenWrapper(const std::filesystem::path &projectRoot) const;
    bool hasLocalMaven() const;
    bool runValidate(const std::filesystem::path &projectRoot, bool useWrapper) const;
};
