#pragma once

#include "cli/routing/CommandRegistry.h"

#include <string>

class CommandRouter
{
public:
    CommandRouter(
        const CommandRegistry &registry,
        std::string version
    );

    int execute(int argc, char *argv[]) const;

private:
    const CommandRegistry &registry_;
    std::string version_;
};