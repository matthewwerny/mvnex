#include "InitCommand.h"

#include "cli/arguments/ArgumentParser.h"
#include "cli/help/HelpPrinter.h"
#include "cli/Style.h"
#include "cli/prompt/Prompt.h"

#include "project/ProjectConfig.h"
#include "project/ProjectGenerator.h"
#include "project/ProjectNaming.h"
#include "project/ProjectValidator.h"
#include "maven/MavenWrapperGenerator.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

static void printSummaryRow(const string &label, const string &value)
{
    cout
        << "  "
        << left
        << setw(10)
        << label
        << value
        << '\n';
}

ProjectConfig InitCommand::collectConfig(const ArgumentParser &parser)
{
    string projectName = parser.getArgument(0);
    string groupId = parser.getOption("group-id");
    string packageName = parser.getOption("package");
    string javaVersion = parser.getOption("java");
    bool hasProjectNameArgument = !parser.getArguments().empty();

    Prompt prompt;

    if (!javaVersion.empty())
    {
        ProjectValidator::validateJavaVersion(javaVersion);
    }

    if (hasProjectNameArgument)
    {
        ProjectValidator::validateProjectName(projectName, javaVersion.empty() ? "21" : javaVersion);
    }

    if (!groupId.empty())
    {
        ProjectValidator::validateGroupId(groupId, javaVersion.empty() ? "21" : javaVersion);
    }

    if (!packageName.empty())
    {
        ProjectValidator::validateGroupId(packageName, javaVersion.empty() ? "21" : javaVersion);
    }

    if (!hasProjectNameArgument)
    {
        projectName = prompt.text(
            "Project name",
            "my-project");

        if (projectName.empty())
        {
            return {};
        }
    }

    if (groupId.empty())
    {
        groupId = prompt.text(
            "Group ID",
            "com.example");

        if (groupId.empty())
        {
            return {};
        }
    }

    if (javaVersion.empty())
    {
        javaVersion = prompt.select(
            "Java version",
            {"8", "11", "17", "21", "25"},
            "21");

        if (javaVersion.empty())
        {
            return {};
        }
    }

    if (packageName.empty())
    {
        packageName =
            groupId + "." +
            ProjectNaming::toPackageName(projectName);
    }

    return {
        projectName,
        groupId,
        packageName,
        javaVersion};
}

int InitCommand::execute(int argc, char *argv[])
{
    ArgumentParser parser(argc, argv, 2);
    parser.addOption({"group-id", 'g', true});
    parser.addOption({"package", 'p', true});
    parser.addOption({"java", 'j', true});
    parser.addOption({"help", 'h', false});
    parser.addOption({"no-wrapper", 'w', false});

    try
    {
        parser.parse();
    }
    catch (const exception &e)
    {
        cerr
            << Style::CYAN
            << "✗ "
            << Style::RESET
            << e.what()
            << '\n';
        return 1;
    }

    if (parser.hasOption("help"))
    {
        HelpPrinter::printInit();
        return 0;
    }

    if (parser.getArguments().size() > 1)
    {
        cerr
            << Style::CYAN
            << "✗ "
            << Style::RESET
            << "Too many arguments. Usage: mvnex init [project-name] [options]\n";
        return 1;
    }

    cout
        << '\n'
        << Style::CYAN
        << "◆"
        << Style::RESET
        << " mvnex init"
        << "\n\n"
        << flush;

    ProjectConfig config;

    try
    {
        config = collectConfig(parser);
    }
    catch (const exception &e)
    {
        cerr 
            << Style::CYAN
            << "✗ " 
            << Style::RESET
            << e.what() 
            << '\n';
        return 1;
    }

    if (config.name.empty() ||
        config.groupId.empty() ||
        config.javaVersion.empty())
    {
        cerr 
            << Style::CYAN
            << "✗ "
            << Style::RESET
            << "Operation cancelled.\n";
        return 1;
    }

    bool skipWrapper = parser.hasOption("no-wrapper");
    bool shouldAskWrapper =
        !skipWrapper &&
        (
            parser.getArguments().empty() ||
            parser.getOption("group-id").empty() ||
            parser.getOption("java").empty()
        );

    if (shouldAskWrapper)
    {
        Prompt prompt;

        string wrapperChoice = prompt.select(
            "Maven Wrapper",
            {"Yes", "No"},
            "Yes"
        );

        if (wrapperChoice.empty())
        {
            cerr 
                << Style::CYAN
                << "✗ "
                << Style::RESET
                << "Operation cancelled.\n";
            return 1;
        }

        skipWrapper = wrapperChoice == "No";
    }

    try
    {
        ProjectValidator::validate(config);

        cout
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

        if (skipWrapper && !MavenWrapperGenerator::isMavenInstalled())
        {
            cout
                << '\n'
                << Style::YELLOW
                << "⚠ "
                << Style::RESET
                << "Maven is not installed.\n"
                << "  You will need Maven to build this project without the wrapper.\n";
        }

        ProjectGenerator::generate(config, skipWrapper);

        cout << "\n✓ Project created successfully\n\n";

        cout << "  cd " << config.name << '\n';
        cout << (skipWrapper ? "  mvn package\n\n" : "  ./mvnw package\n\n");
    }
    catch (const exception &e)
    {
        cerr << "✗ " << e.what() << '\n';
        return 1;
    }

    return 0;
}
