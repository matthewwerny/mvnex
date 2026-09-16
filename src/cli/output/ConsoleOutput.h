#pragma once
#include <string>

class ConsoleOutput
{
public:
    void printVersion(const std::string &version) const;
    void printUnknownCommand(const std::string &commandName) const;
};