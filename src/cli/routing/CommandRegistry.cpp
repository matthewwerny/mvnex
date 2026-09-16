#include "cli/routing/CommandRegistry.h"

#include <stdexcept>

void CommandRegistry::registerCommand(std::shared_ptr<Command> command)
{
    if (!command)
    {
        throw std::invalid_argument("Cannot register a null command.");
    }

    const std::string &name = command->metadata().name;

    if (name.empty())
    {
        throw std::invalid_argument("Cannot register a command without a name.");
    }

    if (find(name))
    {
        throw std::invalid_argument("Command already registered: " + name);
    }

    commands_.push_back(command);
}

std::shared_ptr<Command> CommandRegistry::find(const std::string &name) const
{
    for (const std::shared_ptr<Command> &command : commands_)
    {
        if (command->metadata().name == name)
        {
            return command;
        }
    }

    return nullptr;
}

const std::vector<std::shared_ptr<Command>> &CommandRegistry::commands() const
{
    return commands_;
}
