#pragma once
#include <string>
#include <stdexcept>
#include <vector>

#include "infrastructure/cli/command/CommandMetadata.h"
#include "domain/dependency/ResolvedDependency.h"
#include "domain/project/ProjectConfig.h"

class ConsoleOutput
{
public:
    void printVersion(const std::string &version) const;
    void printUnknownCommand(const std::string &commandName) const;
    void printError(const std::exception &error) const;
    void printError(const std::string &message) const;
    void printLine(const std::string &message) const;
    void printCommandHeader(const CommandMetadata &metadata) const;
    void printProjectCreationSummary(const ProjectConfig &config, bool skipWrapper) const;
    void printWarning(const std::string &message) const;
    void printProjectCreated(const ProjectConfig &config, bool skipWrapper) const;
    void printAddDependencySummary(
        const std::vector<ResolvedDependency> &addedDependencies,
        const std::vector<ResolvedDependency> &skippedDependencies) const;
};
