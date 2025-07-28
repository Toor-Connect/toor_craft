// FileSystemInterface.h
#pragma once
#include <string>

class FileSystemInterface
{
public:
    virtual ~FileSystemInterface() = default;

    virtual void writeFile(const std::string &path, const std::string &content) = 0;
    virtual void readFile(const std::string &path, std::string &outContent) = 0;
};
