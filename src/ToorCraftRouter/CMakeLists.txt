# Define source files for Field library
set(SOURCES
    ToorCraftRouter.cpp
)

add_library(ToorCraftRouterLib STATIC ${SOURCES})

target_link_libraries(ToorCraftRouterLib PRIVATE ToorCraftJSONLib yaml-cpp)

target_include_directories(ToorCraftRouterLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_testing()          # ensures tests can run

include(Catch)            # Catch2 CTest integration

add_executable(ToorCraftRouterTests
    tests/test_ToorCraftRouter.cpp
)

target_link_libraries(ToorCraftRouterTests
    PRIVATE ToorCraftRouterLib
    PRIVATE yaml-cpp
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(ToorCraftRouterTests)