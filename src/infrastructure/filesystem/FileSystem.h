#pragma once

#include <filesystem>
#include <string>

class FileSystem
{
public:
    bool exists(const std::filesystem::path &path) const;
    void createDirectories(const std::filesystem::path &path) const;
    std::string readFile(const std::filesystem::path &path) const;
    void writeFile(const std::filesystem::path &path, const std::string &content) const;
    void addOwnerGroupOthersExecute(const std::filesystem::path &path) const;
};
