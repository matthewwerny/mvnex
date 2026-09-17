#include "infrastructure/filesystem/FileSystem.h"

#include <fstream>
#include <stdexcept>

bool FileSystem::exists(const std::filesystem::path &path) const
{
    return std::filesystem::exists(path);
}

void FileSystem::createDirectories(const std::filesystem::path &path) const
{
    std::filesystem::create_directories(path);
}

void FileSystem::writeFile(const std::filesystem::path &path, const std::string &content) const
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error("Could not create " + path.string());
    }

    file << content;

    if (!file)
    {
        throw std::runtime_error("Could not write " + path.string());
    }
}

void FileSystem::addOwnerGroupOthersExecute(const std::filesystem::path &path) const
{
#ifndef _WIN32
    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_exec |
            std::filesystem::perms::group_exec |
            std::filesystem::perms::others_exec,
        std::filesystem::perm_options::add);
#endif
}
