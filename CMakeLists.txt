cmake_minimum_required(VERSION 3.15)
project(toor_craft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Lua
find_package(Lua REQUIRED)

if (NOT Lua_FOUND)
    message(FATAL_ERROR "Lua not found. Please install Lua development libraries.")
endif()

# Include Lua headers globally
include_directories(${LUA_INCLUDE_DIR})

# Add the Field library as a subdirectory
add_subdirectory(src)

add_subdirectory(examples)