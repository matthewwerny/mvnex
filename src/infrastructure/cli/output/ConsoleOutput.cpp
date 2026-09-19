#include "infrastructure/cli/output/ConsoleOutput.h"
#include "infrastructure/cli/output/Style.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace
{
std::string formatDependency(const ResolvedDependency &dependency)
{
    return dependency.groupId() + ":" +
           dependency.artifactId() + ":" +
           dependency.version();
}

void printSummaryRow(const std::string &label, const std::string &value)
{
    std::cout
        << "  "
        << std::left
        << std::setw(10)
        << label
        << value
        << '\n';
}
}

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

void ConsoleOutput::printError(const std::string &message) const
{
    std::cerr
        << Style::CYAN
        << "✗ "
        << Style::RESET
        << message
        << '\n';
}

void ConsoleOutput::printError(const std::exception &error) const
{
    printError(error.what());
}

void ConsoleOutput::printLine(const std::string &message) const
{
    std::cout << message << '\n';
}

void ConsoleOutput::printCommandHeader(const CommandMetadata &metadata) const
{
    std::cout
        << '\n'
        << Style::CYAN
        << "◆ "
        << Style::RESET
        << "mvnex "
        << metadata.name
        << "\n\n"
        << std::flush;
}

void ConsoleOutput::printProjectCreationSummary(const ProjectConfig &config, bool skipWrapper) const
{
    std::cout
        << '\n'
        << Style::CYAN
        << "◆ Creating Maven project"
        << Style::RESET
        << "\n\n";

    printSummaryRow("Project", config.name);
    printSummaryRow("Group", config.groupId);
    printSummaryRow("Package", config.packageName);
    printSummaryRow("Java", config.javaVersion);
    printSummaryRow("Wrapper", skipWrapper ? "None" : "Maven Wrapper");
}

void ConsoleOutput::printWarning(const std::string &message) const
{
    std::cout
        << '\n'
        << Style::YELLOW
        << "⚠ "
        << Style::RESET
        << message
        << '\n';
}

void ConsoleOutput::printProjectCreated(const ProjectConfig &config, bool skipWrapper) const
{
    std::cout << "\n✓ Project created successfully\n\n";

    std::cout << "  cd " << config.name << '\n';
    std::cout << (skipWrapper ? "  mvn package\n\n" : "  ./mvnw package\n\n");
}

void ConsoleOutput::printAddDependencySummary(
    const std::vector<ResolvedDependency> &addedDependencies,
    const std::vector<ResolvedDependency> &skippedDependencies) const
{
    if (!addedDependencies.empty())
    {
        std::cout
            << '\n'
            << Style::CYAN
            << "◆ "
            << Style::RESET
            << Style::BOLD
            << "Dependencies added"
            << Style::RESET
            << "\n";

        for (const ResolvedDependency &dependency : addedDependencies)
        {
            std::cout
                << Style::DIM
                << "│  "
                << Style::RESET
                << "✓ "
                << formatDependency(dependency)
                << '\n';
        }
    }

    if (!skippedDependencies.empty())
    {
        std::cout
            << '\n'
            << Style::CYAN
            << "◆ "
            << Style::RESET
            << Style::BOLD
            << "Dependencies skipped"
            << Style::RESET
            << "\n";

        for (const ResolvedDependency &dependency : skippedDependencies)
        {
            std::cout
                << Style::DIM
                << "│  "
                << Style::RESET
                << "- "
                << formatDependency(dependency)
                << " already exists or was duplicated in this command"
                << '\n';
        }
    }

    if (addedDependencies.empty() && skippedDependencies.empty())
    {
        std::cout
            << '\n'
            << Style::YELLOW
            << "⚠ "
            << Style::RESET
            << "No dependencies changed.\n";
    }

    std::cout << '\n';
}
