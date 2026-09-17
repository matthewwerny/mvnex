#pragma once
#include "infrastructure/cli/routing/CommandRegistry.h"

#include <string>

class ConsoleOutput;
class HelpPrinter;

class CommandRouter
{
public:
    CommandRouter(
        const CommandRegistry &registry,
        HelpPrinter &helpPrinter,
        ConsoleOutput &consoleOutput,
        std::string version
    );

    int execute(int argc, char *argv[]) const;

private:
    const CommandRegistry &registry_;
    HelpPrinter &helpPrinter_;
    ConsoleOutput &consoleOutput_;
    std::string version_;
};
