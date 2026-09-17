#include "infrastructure/cli/commands/init/InitCommand.h"

#include "infrastructure/cli/arguments/ArgumentParser.h"
#include "infrastructure/cli/output/HelpPrinter.h"
#include "infrastructure/cli/output/ConsoleOutput.h"
#include "infrastructure/cli/prompt/Prompt.h"
#include "infrastructure/cli/commands/init/InitConfigCollector.h"

#include "application/init/InitUseCase.h"
#include "domain/project/ProjectConfig.h"

#include <stdexcept>

InitCommand::InitCommand(
    HelpPrinter &helpPrinter,
    ConsoleOutput &consoleOutput,
    Prompt &prompt,
    InitConfigCollector &configCollector,
    InitUseCase &useCase)
    : helpPrinter_(helpPrinter),
      consoleOutput_(consoleOutput),
      prompt_(prompt),
      configCollector_(configCollector),
      useCase_(useCase)
{
    metadata_.addOption({"group-id",
                         'g',
                         "id",
                         "Set Maven groupId",
                         true});

    metadata_.addOption({"package",
                         'p',
                         "name",
                         "Set Java package name",
                         true});

    metadata_.addOption({"java",
                         'j',
                         "version",
                         "Set Java version",
                         true});

    metadata_.addOption({"no-wrapper",
                         0,
                         "",
                         "Do not generate Maven Wrapper",
                         false});

    metadata_.addOption({"help",
                         'h',
                         "",
                         "Show init help",
                         false});

    metadata_.addExample("mvnex init");
    metadata_.addExample("mvnex init my-app");
    metadata_.addExample("mvnex init my-app --java 21");
    metadata_.addExample("mvnex init my-app --group-id com.example --package com.example.app");
    metadata_.addExample("mvnex init my-app --no-wrapper");
}

const CommandMetadata &InitCommand::metadata() const
{
    return metadata_;
}

int InitCommand::execute(int argc, char *argv[])
{
    ArgumentParser parser(argc, argv, 2);

    ProjectConfig config;

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

    if (parser.getArguments().size() > 1)
    {
        consoleOutput_.printError("Too many arguments. Usage: " + metadata_.usage);
        return 1;
    }

    consoleOutput_.printCommandHeader(metadata_);

    try
    {
        config = configCollector_.collectConfig(parser);
    }
    catch (const std::exception &e)
    {
        consoleOutput_.printError(e);
        return 1;
    }

    if (config.name.empty() ||
        config.groupId.empty() ||
        config.javaVersion.empty())
    {
        consoleOutput_.printError("Operation cancelled.");
        return 1;
    }

    bool skipWrapper = parser.hasOption("no-wrapper");
    bool shouldAskWrapper =
        !skipWrapper &&
        (parser.getArguments().empty() ||
         parser.getOption("group-id").empty() ||
         parser.getOption("java").empty());

    if (shouldAskWrapper)
    {
        std::string wrapperChoice = prompt_.select(
            "Maven Wrapper",
            {"Yes", "No"},
            "Yes");

        if (wrapperChoice.empty())
        {
            consoleOutput_.printError("Operation cancelled.");
            return 1;
        }

        skipWrapper = wrapperChoice == "No";
    }

    consoleOutput_.printProjectCreationSummary(config, skipWrapper);

    try
    {
        InitUseCaseResult result = useCase_.execute({config, skipWrapper});

        if (result.mavenMissing)
        {
            consoleOutput_.printWarning(
                "Maven is not installed.\n  You will need Maven to build this project without the wrapper.");
        }

        consoleOutput_.printProjectCreated(config, skipWrapper);
    }
    catch (const std::exception &e)
    {
        consoleOutput_.printError(e);
        return 1;
    }

    return 0;
}
