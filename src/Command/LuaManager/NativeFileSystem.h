#pragma once
#include "FileSystemInterface.h"
#include <fstream>

class NativeFileSystem : public FileSystemInterface
{
public:
    void writeFile(const std::string &path, const std::string &content) override;
    void readFile(const std::string &path, std::string &outContent) override;
};
