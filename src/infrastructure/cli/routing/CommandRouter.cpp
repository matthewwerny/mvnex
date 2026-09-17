#include "infrastructure/cli/routing/CommandRouter.h"
#include "infrastructure/cli/output/HelpPrinter.h"
#include "infrastructure/cli/output/ConsoleOutput.h"
#include <utility>

CommandRouter::CommandRouter(
    const CommandRegistry &registry,
    HelpPrinter &helpPrinter,
    ConsoleOutput &consoleOutput,
    std::string version)
    : registry_(registry),
      helpPrinter_(helpPrinter),
      consoleOutput_(consoleOutput),
      version_(std::move(version))
{
}

int CommandRouter::execute(int argc, char *argv[]) const
{
    if (argc < 2)
    {
        helpPrinter_.printGlobalHelp(registry_.commands());
        return 0;
    }

    std::string commandName = argv[1];

    if (commandName == "--help" || commandName == "-h")
    {
        helpPrinter_.printGlobalHelp(registry_.commands());
        return 0;
    }

    if (commandName == "--version" || commandName == "-v")
    {
        consoleOutput_.printVersion(version_);
        return 0;
    }

    std::shared_ptr<Command> command = registry_.find(commandName);

    if (!command)
    {
        consoleOutput_.printUnknownCommand(commandName);
        return 1;
    }

    return command->execute(argc, argv);
}
