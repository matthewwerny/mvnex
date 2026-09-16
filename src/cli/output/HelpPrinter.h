#pragma once

#include "cli/routing/Command.h"

#include <memory>
#include <vector>

class HelpPrinter
{
public:
    void printGlobalHelp(
        const std::vector<std::shared_ptr<Command>> &commands) const;
    static void printInit();
};
