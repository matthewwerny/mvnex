#pragma once
#include "project/ProjectConfig.h"
#include "cli/routing/Command.h"

class ArgumentParser;

class InitCommand : public Command
{
public:
    const CommandMetadata &metadata() const override;
    int execute(int argc, char *argv[]) override;

private:
    ProjectConfig collectConfig(const ArgumentParser &parser);
    CommandMetadata metadata_{
        "init",
        "Create a new Maven project",
        "mvnex init [project-name] [options]"};
};
