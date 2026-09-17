#pragma once
#include <string>

struct CommandOption{
    std::string longName;
    char shortName;
    std::string valueName;
    std::string description;
    bool requiresValue;
};