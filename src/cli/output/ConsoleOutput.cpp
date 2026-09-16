#include "cli/output/ConsoleOutput.h"
#include "cli/output/Style.h"
#include <string>
#include <iostream>

void ConsoleOutput::printVersion(const std::string &version) const
{
    std::cout
        << Style::CYAN
        << "◆ "
        << Style::RESET
        << Style::BOLD
        << "mvnex "
        << Style::RESET
        << version
        << '\n';
}

void ConsoleOutput::printUnknownCommand(const std::string &commandName) const
{
    std::cerr
        << Style::CYAN
        << "✗ "
        << Style::RESET
        << "Command not found: "
        << commandName
        << '\n';
}