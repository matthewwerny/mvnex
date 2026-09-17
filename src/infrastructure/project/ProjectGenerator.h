#pragma once

#include "application/ports/ProjectCreator.h"

class FileSystem;
class MavenWrapperGenerator;

class ProjectGenerator : public ProjectCreator
{
public:
    ProjectGenerator(
        const FileSystem &fileSystem,
        MavenWrapperGenerator &mavenWrapperGenerator);

    void create(const ProjectConfig &config, bool skipWrapper) const override;

private:
    const FileSystem &fileSystem_;
    MavenWrapperGenerator &mavenWrapperGenerator_;
};
