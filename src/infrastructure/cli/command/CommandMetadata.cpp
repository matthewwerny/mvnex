#include "infrastructure/cli/command/CommandMetadata.h"
#include <stdexcept>

namespace
{
    const CommandOption *findOptionBy(
        const std::vector<CommandOption> &options,
        const std::string &name,
        const std::string CommandOption::*field)
    {
        for (const CommandOption &option : options)
        {
            if (option.*field == name)
            {
                return &option;
            }
        }

        return nullptr;
    }
}

void CommandMetadata::addOption(const CommandOption &option)
{
    if (option.longName.empty())
    {
        throw std::invalid_argument("Command option requires a long name.");
    }

    if (findLongOption(option.longName))
    {
        throw std::invalid_argument("Command option already registered: " + option.longName);
    }

    if (option.shortName != 0 && findShortOption(option.shortName))
    {
        throw std::invalid_argument("Command short option already registered.");
    }

    options.push_back(option);
}

void CommandMetadata::addExample(const std::string &example)
{
    if (example.empty())
    {
        throw std::invalid_argument("Example cannot be empty.");
    }

    examples.push_back(example);
}

const CommandOption *CommandMetadata::findLongOption(const std::string &name) const
{
    return findOptionBy(options, name, &CommandOption::longName);
}

const CommandOption *CommandMetadata::findShortOption(char name) const
{
    for (const CommandOption &option : options)
    {
        if (option.shortName == name)
        {
            return &option;
        }
    }

    return nullptr;
}
