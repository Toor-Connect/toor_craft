# Define source files for Field library
set(SOURCES
    ToorCraftAPI.cpp
)

add_library(ToorCraftAPILib STATIC ${SOURCES})

target_link_libraries(ToorCraftAPILib PUBLIC ToorCraftEngineLib)
target_link_libraries(ToorCraftAPILib PRIVATE EntityLib yaml-cpp)

target_include_directories(ToorCraftAPILib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_testing()          # ensures tests can run

include(Catch)            # Catch2 CTest integration

add_executable(ToorCraftAPITests
    tests/test_ToorCraftAPI.cpp
)

target_link_libraries(ToorCraftAPITests
    PRIVATE ToorCraftAPILib
    PRIVATE yaml-cpp
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(ToorCraftAPITests)