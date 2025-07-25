// FileSystemFactory.h
#pragma once
#include <memory>
#include "FileSystemInterface.h"

std::unique_ptr<FileSystemInterface> createFileSystem();
