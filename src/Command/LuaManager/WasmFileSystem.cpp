#include "WasmFileSystem.h"
#include <emscripten.h>

extern "C" {
    // Declare JS functions that will be implemented in JS glue code
    void js_write_file(const char* path, const char* content);
    const char* js_read_file(const char* path);
}

bool WasmFileSystem::writeFile(const std::string& path, const std::string& content, std::string& error) {
    // No native error handling here, delegate to JS
    js_write_file(path.c_str(), content.c_str());
    return true;
}

bool WasmFileSystem::readFile(const std::string& path, std::string& outContent, std::string& error) {
    const char* result = js_read_file(path.c_str());
    if (!result) {
        error = "JS read_file returned null for path: " + path;
        return false;
    }
    outContent = std::string(result);
    return true;
}
