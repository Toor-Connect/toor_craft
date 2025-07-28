#pragma once
#include "FileSystemInterface.h"
#include <string>

// WASM version delegates file I/O to JS via extern "C" hooks
class WasmFileSystem : public FileSystemInterface
{
public:
    void writeFile(const std::string &path, const std::string &content) override;
    void readFile(const std::string &path, std::string &outContent) override;
};
