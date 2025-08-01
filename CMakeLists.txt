cmake_minimum_required(VERSION 3.15)
project(toor_craft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ✅ Detect Emscripten automatically
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    message(STATUS "🚀 Building for WebAssembly (Emscripten detected)")
endif()

include(FetchContent)

# ✅ Enable CTest (for test discovery)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

set(INJA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(INJA_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

# ✅ Fetch Inja (templating library)
FetchContent_Declare(
    inja
    GIT_REPOSITORY https://github.com/pantor/inja.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(inja)

# ✅ Fetch Lua (CMake version)
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua.git
    GIT_TAG v5.4.5
)
FetchContent_MakeAvailable(lua)


FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)

FetchContent_MakeAvailable(yaml-cpp)


# 🔽 Fetch Catch2
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0 # (or latest stable)
)

FetchContent_MakeAvailable(catch2)

# ✅ Lua is now provided as a proper CMake target
# We don’t need to manually list sources — the repo handles it
# It will build `lua` static lib by default

# ✅ Global include dirs for all code
include_directories(${inja_SOURCE_DIR}/single_include)
include_directories(${inja_SOURCE_DIR}/third_party/include)


# ✅ Add the Field library and other modules
add_subdirectory(src)

# ✅ Add examples (test/demo executables)
add_subdirectory(examples)
