#pragma once

#include "domain/project/ProjectConfig.h"

class ProjectCreator
{
public:
    virtual ~ProjectCreator() = default;

    virtual void create(const ProjectConfig &config, bool skipWrapper) const = 0;
};
