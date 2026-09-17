#pragma once
#include "domain/project/ProjectConfig.h"

class ArgumentParser;
class Prompt;

class InitConfigCollector
{
public:
    explicit InitConfigCollector(Prompt &prompt);

    ProjectConfig collectConfig(const ArgumentParser &parser) const;

private:
    Prompt &prompt_;
};
