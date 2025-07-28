#include "NativeFileSystem.h"
#include <stdexcept>
#include <fstream>
#include <sstream>

std::filesystem::path NativeFileSystem::resolvePath(const std::string &path) const
{
    std::filesystem::path p(path);
    if (p.is_absolute())
    {
        return p;
    }
    if (!basePath_.empty())
    {
        return basePath_ / p;
    }
    return std::filesystem::current_path() / p;
}

void NativeFileSystem::writeFile(const std::string &path, const std::string &content)
{
    try
    {
        auto resolvedPath = resolvePath(path);
        std::ofstream ofs(resolvedPath);
        if (!ofs)
        {
            throw std::runtime_error("Failed to open file for writing: " + path);
        }
        ofs << content;
    }
    catch (const std::exception &ex)
    {
        throw std::runtime_error("Exception while writing file: " + std::string(ex.what()));
    }
}

void NativeFileSystem::readFile(const std::string &path, std::string &outContent)
{
    try
    {
        auto resolved = resolvePath(path);
        std::ifstream ifs(resolved);
        if (!ifs)
        {
            throw std::runtime_error("Failed to open file for reading: " + path);
        }
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        outContent = buffer.str();
    }
    catch (const std::exception &ex)
    {
        throw std::runtime_error("Exception while reading file: " + std::string(ex.what()));
    }
}
