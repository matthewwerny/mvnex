#pragma once

#include "application/ports/MavenChecker.h"
#include "application/ports/ProjectCreator.h"
#include "domain/project/ProjectConfig.h"

struct InitUseCaseRequest
{
    ProjectConfig config;
    bool skipWrapper;
};

struct InitUseCaseResult
{
    bool mavenMissing;
};

class InitUseCase
{
public:
    InitUseCase(
        const ProjectCreator &projectCreator,
        const MavenChecker &mavenChecker);

    InitUseCaseResult execute(const InitUseCaseRequest &request) const;

private:
    const ProjectCreator &projectCreator_;
    const MavenChecker &mavenChecker_;
};
