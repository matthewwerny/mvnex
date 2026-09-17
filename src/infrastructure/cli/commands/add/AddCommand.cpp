#include "infrastructure/cli/commands/add/AddCommand.h"

#include "infrastructure/cli/arguments/ArgumentParser.h"
#include "infrastructure/cli/output/ConsoleOutput.h"
#include "infrastructure/cli/output/HelpPrinter.h"

#include <stdexcept>

AddCommand::AddCommand(
    HelpPrinter &helpPrinter,
    ConsoleOutput &consoleOutput)
    : helpPrinter_(helpPrinter),
      consoleOutput_(consoleOutput)
{
    metadata_.addOption({"help",
                         'h',
                         "",
                         "Show add help",
                         false});

    metadata_.addExample("mvnex add lombok");
    metadata_.addExample("mvnex add lombok:1.18.48");
    metadata_.addExample("mvnex add org.postgresql:postgresql");
    metadata_.addExample("mvnex add org.projectlombok:lombok:1.18.48");
}

const CommandMetadata &AddCommand::metadata() const
{
    return metadata_;
}

int AddCommand::execute(int argc, char *argv[])
{
    ArgumentParser parser(argc, argv, 2);
    for (const CommandOption &option : metadata_.options)
    {
        parser.addOption({option.longName, option.shortName, option.requiresValue});
    }

    try
    {
        parser.parse();
    }
    catch (const std::exception &e)
    {
        consoleOutput_.printError(e);
        return 1;
    }

    if (parser.hasOption("help"))
    {
        helpPrinter_.printCommandHelp(*this);
        return 0;
    }

    const std::vector<std::string> &dependencies = parser.getArguments();

    if (dependencies.empty())
    {
        consoleOutput_.printError("Missing dependency. Usage: " + metadata_.usage);
        return 1;
    }

    for (const std::string &dependency : dependencies)
    {
        consoleOutput_.printLine(dependency);
    }

    return 0;
}
