#include "NativeFileSystem.h"
#include <stdexcept>
#include <fstream>
#include <sstream>

void NativeFileSystem::writeFile(const std::string &path, const std::string &content)
{
    try
    {
        std::ofstream ofs(path);
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
        std::ifstream ifs(path);
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
