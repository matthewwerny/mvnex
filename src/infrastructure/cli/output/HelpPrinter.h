#pragma once

#include "infrastructure/cli/command/Command.h"

#include <memory>
#include <vector>

class HelpPrinter
{
public:
    void printGlobalHelp(
        const std::vector<std::shared_ptr<Command>> &commands) const;

    void printCommandHelp(const Command &command) const;
};
