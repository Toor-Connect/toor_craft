#include "WasmFileSystem.h"
#include <emscripten.h>
#include <string>
#include <stdexcept>

extern "C"
{
    // Declare JS functions that will be implemented in JS glue code
    void js_write_file(const char *path, const char *content);
    const char *js_read_file(const char *path);
}

void WasmFileSystem::writeFile(const std::string &path, const std::string &content)
{
    // No native error handling here, delegate to JS
    js_write_file(path.c_str(), content.c_str());
    return true;
}

void WasmFileSystem::readFile(const std::string &path, std::string &outContent)
{
    const char *result = js_read_file(path.c_str());
    if (!result)
    {
        throw std::runtime_error("JS read_file returned null for path: " + path);
    }
    outContent = std::string(result);
}
