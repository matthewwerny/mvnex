#pragma once
#include <string>
#include "cli/routing/CommandMetadata.h"

class Command
{
public:
    virtual ~Command() = default;

    virtual const CommandMetadata &metadata() const = 0;
    virtual int execute(int argc, char *argv[]) = 0;
};