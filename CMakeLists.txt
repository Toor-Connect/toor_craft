cmake_minimum_required(VERSION 3.15)
project(toor_craft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Lua
find_package(Lua REQUIRED)

if (NOT Lua_FOUND)
    message(FATAL_ERROR "Lua not found. Please install Lua development libraries.")
endif()

include(FetchContent)

FetchContent_Declare(
    inja
    GIT_REPOSITORY https://github.com/pantor/inja.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(inja)

# Include Lua headers globally
include_directories(${LUA_INCLUDE_DIR})
include_directories(${inja_SOURCE_DIR}/single_include)
include_directories(${inja_SOURCE_DIR}/third_party/include)

# Add the Field library as a subdirectory
add_subdirectory(src)

add_subdirectory(examples)