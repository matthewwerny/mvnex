#include "cli/output/HelpPrinter.h"

#include "cli/output/Style.h"

#include <iostream>
#include <iomanip>

void HelpPrinter::printGlobalHelp(const std::vector<std::shared_ptr<Command>> &commands) const
{
    std::cout
        << "\n"
        << Style::CYAN
        << "◆ "
        << Style::RESET
        << Style::BOLD
        << "mvnex"
        << Style::RESET
        << "\n\n"

        << "  A modern developer experience for Maven.\n\n"

        << Style::CYAN
        << "◇ "
        << Style::RESET
        << Style::BOLD
        << "Usage"
        << Style::RESET
        << "\n"

        << Style::DIM
        << "│"
        << Style::RESET
        << "\n"

        << Style::DIM
        << "│  "
        << Style::RESET
        << "mvnex <command> [options]\n"

        << Style::DIM
        << "│"
        << Style::RESET
        << "\n\n"

        << Style::CYAN
        << "◇ "
        << Style::RESET
        << Style::BOLD
        << "Commands"
        << Style::RESET
        << "\n"

        << Style::DIM
        << "│"
        << Style::RESET
        << "\n";

    for (const std::shared_ptr<Command> &command : commands)
    {
        std::cout
            << Style::DIM
            << "│  "
            << Style::RESET
            << Style::CYAN
            << std::left
            << std::setw(10)
            << command->metadata().name
            << Style::RESET
            << command->metadata().description
            << "\n";
    }

    std::cout
        << Style::DIM
        << "│"
        << Style::RESET
        << "\n\n"

        << Style::CYAN
        << "◇ "
        << Style::RESET
        << Style::BOLD
        << "Options"
        << Style::RESET
        << "\n"

        << Style::DIM
        << "│"
        << Style::RESET
        << "\n"

        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "-h, --help"
        << Style::RESET
        << "       Show help\n"

        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "-v, --version"
        << Style::RESET
        << "    Show version\n"

        << Style::DIM
        << "│"
        << Style::RESET
        << "\n\n";
}

void HelpPrinter::printInit()
{
    std::cout
        << "\n"
        << Style::CYAN
        << "◆ "
        << Style::RESET
        << Style::BOLD
        << "mvnex init"
        << Style::RESET
        << "\n\n"

        << "  Create a new Maven project.\n\n"

        << Style::CYAN
        << "◇ "
        << Style::RESET
        << Style::BOLD
        << "Usage"
        << Style::RESET
        << "\n"
        << Style::DIM
        << "│  "
        << Style::RESET
        << "mvnex init [project-name] [options]\n\n"

        << Style::CYAN
        << "◇ "
        << Style::RESET
        << Style::BOLD
        << "Options"
        << Style::RESET
        << "\n"
        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "-g, --group-id <id>"
        << Style::RESET
        << "    Set Maven groupId\n"
        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "-p, --package <name>"
        << Style::RESET
        << "   Set Java package name\n"
        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "-j, --java <version>"
        << Style::RESET
        << "    Set Java version\n"
        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "--no-wrapper"
        << Style::RESET
        << "             Do not generate Maven Wrapper\n"
        << Style::DIM
        << "│  "
        << Style::RESET
        << Style::CYAN
        << "-h, --help"
        << Style::RESET
        << "              Show init help\n\n";
}
