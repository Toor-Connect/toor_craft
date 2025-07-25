// FileSystemInterface.h
#pragma once
#include <string>

class FileSystemInterface {
public:
    virtual ~FileSystemInterface() = default;

    virtual bool writeFile(const std::string& path, const std::string& content, std::string& error) = 0;
    virtual bool readFile(const std::string& path, std::string& outContent, std::string& error) = 0;
};
