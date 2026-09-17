#include "infrastructure/cli/output/HelpPrinter.h"

#include "infrastructure/cli/output/Style.h"

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

void HelpPrinter::printCommandHelp(const Command &command) const
{
    const CommandMetadata &metadata = command.metadata();

    std::cout
        << "\n"
        << Style::CYAN
        << "◆ "
        << Style::RESET
        << Style::BOLD
        << "mvnex "
        << metadata.name
        << Style::RESET
        << "\n\n"

        << "  "
        << metadata.description
        << "\n\n"

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
        << metadata.usage
        << "\n\n";

    if (!metadata.options.empty())
    {
        std::cout
            << Style::CYAN
            << "◇ "
            << Style::RESET
            << Style::BOLD
            << "Options"
            << Style::RESET
            << "\n";

        for (const CommandOption &option : metadata.options)
        {
            std::string renderedOption;

            if (option.shortName != 0)
            {
                renderedOption += "-";
                renderedOption += option.shortName;
                renderedOption += ", ";
            }

            renderedOption += "--";
            renderedOption += option.longName;

            if (!option.valueName.empty())
            {
                renderedOption += " <";
                renderedOption += option.valueName;
                renderedOption += ">";
            }

            std::cout
                << Style::DIM
                << "│  "
                << Style::RESET
                << Style::CYAN
                << std::left
                << std::setw(28)
                << renderedOption
                << Style::RESET
                << option.description
                << "\n";
        }

        std::cout << "\n";
    }

    if (!metadata.examples.empty())
    {
        std::cout
            << Style::CYAN
            << "◇ "
            << Style::RESET
            << Style::BOLD
            << "Examples"
            << Style::RESET
            << "\n";

        for (const std::string &example : metadata.examples)
        {
            std::cout
                << Style::DIM
                << "│  "
                << Style::RESET
                << example
                << "\n";
        }

        std::cout << "\n";
    }
}
