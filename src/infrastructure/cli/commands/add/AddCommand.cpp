#include "infrastructure/cli/commands/add/AddCommand.h"

#include "infrastructure/cli/arguments/ArgumentParser.h"
#include "infrastructure/cli/output/ConsoleOutput.h"
#include "infrastructure/cli/output/HelpPrinter.h"
#include "infrastructure/cli/dependency/DependencyArgumentsParser.h"
#include "infrastructure/cli/prompt/Prompt.h"

#include "application/add/AddUseCase.h"
#include "application/errors/DependencyNotFound.h"
#include "application/errors/DependencyResolverUnavailable.h"
#include "application/errors/MultipleDependencyMatches.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
const std::string SearchAgainOption = "Search again...";

std::string formatDependency(const ResolvedDependency &dependency)
{
    return dependency.groupId() + ":" +
           dependency.artifactId() + ":" +
           dependency.version();
}

std::vector<std::string> candidateLabels(
    const std::vector<ResolvedDependency> &candidates)
{
    std::vector<std::string> labels;
    const std::size_t visibleCandidates = std::min<std::size_t>(3, candidates.size());

    labels.reserve(visibleCandidates);

    for (std::size_t i = 0; i < visibleCandidates; ++i)
    {
        labels.push_back(formatDependency(candidates[i]));
    }

    labels.push_back(SearchAgainOption);

    return labels;
}
}

AddCommand::AddCommand(
    HelpPrinter &helpPrinter,
    ConsoleOutput &consoleOutput,
    DependencyArgumentsParser &dependencyArgumentsParser,
    Prompt &prompt,
    AddUseCase &addUseCase)
    : helpPrinter_(helpPrinter),
      consoleOutput_(consoleOutput),
      dependencyArgumentsParser_(dependencyArgumentsParser),
      prompt_(prompt),
      addUseCase_(addUseCase)
{
    metadata_.addOption({"help",
                         'h',
                         "",
                         "Show add help",
                         false});

    metadata_.addOption({"version",
                         'v',
                         "version",
                         "Set dependency version",
                         true});

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

    if (parser.hasOption("version") && dependencies.size() > 1)
    {
        consoleOutput_.printError("--version can only be used when adding a single dependency.");
        return 1;
    }

    std::vector<DependencyRequest> parsedDependencies;

    try
    {
        parsedDependencies = dependencyArgumentsParser_.parse(dependencies, parser.getOption("version"));
    }
    catch (const std::exception &e)
    {
        consoleOutput_.printError(e);
        return 1;
    }

    std::vector<DependencyRequest> pendingDependencies = parsedDependencies;
    AddUseCaseResponse response;
    bool completed = false;

    do
    {
        try
        {
            response = addUseCase_.execute({pendingDependencies});
            completed = true;
        }
        catch (const MultipleDependencyMatches &error)
        {
            if (error.dependencyIndex() == MultipleDependencyMatches::UnknownDependencyIndex ||
                error.dependencyIndex() >= pendingDependencies.size())
            {
                consoleOutput_.printError(error);
                return 1;
            }

            std::vector<std::string> options = candidateLabels(error.candidates());

            if (options.empty())
            {
                consoleOutput_.printError(error);
                return 1;
            }

            std::string selectedLabel = prompt_.select(
                "Select dependency for " + error.query(),
                options);

            if (selectedLabel.empty())
            {
                consoleOutput_.printError("Operation cancelled.");
                return 1;
            }

            if (selectedLabel == SearchAgainOption)
            {
                std::string searchTerm = prompt_.text(
                    "Search dependency",
                    error.query(),
                    true);

                if (searchTerm.empty())
                {
                    consoleOutput_.printError("Operation cancelled.");
                    return 1;
                }

                pendingDependencies[error.dependencyIndex()] =
                    DependencyRequest::searchTerm(searchTerm);
                continue;
            }

            auto selected = std::find(options.begin(), options.end(), selectedLabel);

            if (selected == options.end())
            {
                consoleOutput_.printError("Invalid selection.");
                return 1;
            }

            const ResolvedDependency &dependency =
                error.candidates()[static_cast<std::size_t>(selected - options.begin())];

            pendingDependencies[error.dependencyIndex()] =
                DependencyRequest::coordinateWithVersion(
                    dependency.groupId(),
                    dependency.artifactId(),
                    dependency.version());
        }
        catch (const DependencyNotFound &error)
        {
            consoleOutput_.printError(error);
            return 1;
        }
        catch (const DependencyResolverUnavailable &error)
        {
            consoleOutput_.printError(error);
            return 1;
        }
        catch (const std::exception &error)
        {
            consoleOutput_.printError(error);
            return 1;
        }
    } while (!completed);

    consoleOutput_.printAddDependencySummary(
        response.addedDependencies,
        response.skippedDependencies);

    return 0;
}
