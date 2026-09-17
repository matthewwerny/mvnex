#pragma once

#include "infrastructure/cli/command/Command.h"

#include <memory>
#include <string>
#include <vector>

class CommandRegistry
{
public:
    void registerCommand(std::shared_ptr<Command> command);

    std::shared_ptr<Command> find(const std::string &name) const;

    const std::vector<std::shared_ptr<Command>> &commands() const;

private:
    std::vector<std::shared_ptr<Command>> commands_;
};