# Define source files for Field library
set(SOURCES
    ToorCraftJSON.cpp
)

add_library(ToorCraftJSONLib STATIC ${SOURCES})

target_link_libraries(ToorCraftJSONLib PUBLIC ToorCraftEngineLib)
target_link_libraries(ToorCraftJSONLib PRIVATE EntityLib yaml-cpp)

target_include_directories(ToorCraftJSONLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_testing()          # ensures tests can run

include(Catch)            # Catch2 CTest integration

add_executable(ToorCraftJSONTests
    tests/test_ToorCraftJSON.cpp
)

target_link_libraries(ToorCraftJSONTests
    PRIVATE ToorCraftJSONLib
    PRIVATE yaml-cpp
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(ToorCraftJSONTests)