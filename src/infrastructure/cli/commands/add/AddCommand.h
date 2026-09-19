#pragma once

#include "infrastructure/cli/command/Command.h"

class ConsoleOutput;
class HelpPrinter;
class DependencyArgumentsParser;
class Prompt;
class AddUseCase;

class AddCommand : public Command
{
public:
    AddCommand(
        HelpPrinter &helpPrinter,
        ConsoleOutput &consoleOutput,
        DependencyArgumentsParser &dependencyArgumentsParser,
        Prompt &prompt,
        AddUseCase &addUseCase);

    const CommandMetadata &metadata() const override;
    int execute(int argc, char *argv[]) override;

private:
    HelpPrinter &helpPrinter_;
    ConsoleOutput &consoleOutput_;
    DependencyArgumentsParser &dependencyArgumentsParser_;
    Prompt &prompt_;
    AddUseCase &addUseCase_;

    CommandMetadata metadata_{
        "add",
        "Add a dependency to the project",
        "mvnex add <dependency> [options]"};
};
