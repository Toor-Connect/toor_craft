# Define source files for Field library
set(SOURCES
    ToorCraftEngine.cpp
)

add_library(ToorCraftEngineLib STATIC ${SOURCES})

target_link_libraries(ToorCraftEngineLib PRIVATE SchemaManagerLib EntityManagerLib)

target_include_directories(ToorCraftEngineLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_testing()          # ensures tests can run

include(Catch)            # Catch2 CTest integration

add_executable(ToorCraftTests
    tests/test_ToorCraftEngine.cpp
    tests/test_ToorCraftEngine_Errors.cpp
)

target_link_libraries(ToorCraftTests
    PRIVATE ToorCraftEngineLib
    FieldValueLib
    EntityLib
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(ToorCraftTests)