#pragma once
#include "infrastructure/cli/command/Command.h"

class ConsoleOutput;
class HelpPrinter;
class InitConfigCollector;
class InitUseCase;
class Prompt;

class InitCommand : public Command
{
public:
    InitCommand(
        HelpPrinter &helpPrinter,
        ConsoleOutput &consoleOutput,
        Prompt &prompt,
        InitConfigCollector &configCollector,
        InitUseCase &useCase);

    const CommandMetadata &metadata() const override;
    int execute(int argc, char *argv[]) override;

private:
    HelpPrinter &helpPrinter_;
    ConsoleOutput &consoleOutput_;
    Prompt &prompt_;
    InitConfigCollector &configCollector_;
    InitUseCase &useCase_;

    CommandMetadata metadata_{
        "init",
        "Create a new Maven project",
        "mvnex init [project-name] [options]"};
};
