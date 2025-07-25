# Base source files that are always compiled
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager/LuaManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager/FileSystemFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaCommand.cpp
)

# ✅ Add the correct filesystem implementation based on target
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager/WasmFileSystem.cpp)
else()
    list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager/NativeFileSystem.cpp)
endif()

# ✅ Build the library
add_library(CommandLib STATIC ${SOURCES})

# ✅ Link dependencies
target_link_libraries(CommandLib
    PUBLIC EntityLib
    PRIVATE EntityManagerLib lua_static
)

# ✅ Include your own headers only (not lua)
target_include_directories(CommandLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager
)
