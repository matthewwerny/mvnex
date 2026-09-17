#pragma once

#include "infrastructure/cli/command/CommandOption.h"

#include <string>
#include <vector>

class CommandMetadata
{
public:
    std::string name;
    std::string description;
    std::string usage;
    std::vector<CommandOption> options;
    std::vector<std::string> examples;

    void addOption(const CommandOption &option);
    void addExample(const std::string &example);

    const CommandOption *findLongOption(const std::string &name) const;
    const CommandOption *findShortOption(char name) const;
};
