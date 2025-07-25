# Define source files for Field library
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager/LuaManager.cpp
    LuaCommand.cpp
)

add_library(CommandLib STATIC ${SOURCES})

target_link_libraries(CommandLib PUBLIC EntityLib)
target_link_libraries(CommandLib PRIVATE EntityManagerLib ${LUA_LIBRARIES})

target_include_directories(CommandLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaManager
    ${LUA_INCLUDE_DIRS}
)