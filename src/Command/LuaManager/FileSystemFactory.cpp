// FileSystemFactory.cpp
#include "FileSystemFactory.h"
#include "NativeFileSystem.h"
#include "WasmFileSystem.h"

std::unique_ptr<FileSystemInterface> createFileSystem() {
#ifdef __EMSCRIPTEN__
    return std::make_unique<WasmFileSystem>();
#else
    return std::make_unique<NativeFileSystem>();
#endif
}
