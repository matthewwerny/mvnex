#include "cli/routing/CommandRouter.h"
#include "cli/output/HelpPrinter.h"
#include "cli/output/ConsoleOutput.h"
#include <utility>

CommandRouter::CommandRouter(
    const CommandRegistry &registry,
    std::string version)
    : registry_(registry),
      version_(std::move(version))
{
}

int CommandRouter::execute(int argc, char *argv[]) const
{
    HelpPrinter helpPrinter;
    ConsoleOutput consoleOutput;
    if (argc < 2)
    {
        helpPrinter.printGlobalHelp(registry_.commands());
        return 0;
    }

    std::string commandName = argv[1];

    if (commandName == "--help" || commandName == "-h")
    {
        helpPrinter.printGlobalHelp(registry_.commands());
        return 0;
    }

    if (commandName == "--version" || commandName == "-v")
    {
        consoleOutput.printVersion(version_);
        return 0;
    }

    std::shared_ptr<Command> command = registry_.find(commandName);

    if (!command)
    {
        consoleOutput.printUnknownCommand(commandName);
        return 1;
    }

    return command->execute(argc, argv);
}