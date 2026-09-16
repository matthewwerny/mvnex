#pragma once

#include "cli/routing/Command.h"

class AddCommand : public Command
{
public:
    const CommandMetadata &metadata() const override;
    int execute(int argc, char *argv[]) override;

private:
    CommandMetadata metadata_{
        "add",
        "Add a dependency to the project",
        "mvnex add <dependency> [options]"};
};
