#pragma once
#include "FileSystemInterface.h"
#include <string>

// WASM version delegates file I/O to JS via extern "C" hooks
class WasmFileSystem : public FileSystemInterface {
public:
    bool writeFile(const std::string& path, const std::string& content, std::string& error) override;
    bool readFile(const std::string& path, std::string& outContent, std::string& error) override;
};
