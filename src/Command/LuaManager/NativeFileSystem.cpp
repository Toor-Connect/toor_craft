#include "NativeFileSystem.h"
#include <sstream>

bool NativeFileSystem::writeFile(const std::string& path, const std::string& content, std::string& error) {
    std::ofstream ofs(path);
    if (!ofs) {
        error = "Failed to open file for writing: " + path;
        return false;
    }
    ofs << content;
    return true;
}

bool NativeFileSystem::readFile(const std::string& path, std::string& outContent, std::string& error) {
    std::ifstream ifs(path);
    if (!ifs) {
        error = "Failed to open file for reading: " + path;
        return false;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    outContent = buffer.str();
    return true;
}
