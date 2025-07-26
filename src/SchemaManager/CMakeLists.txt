# =======================
# SchemaManager library
# =======================
set(SOURCES
    SchemaManager.cpp
)

add_library(SchemaManagerLib STATIC ${SOURCES})

target_link_libraries(SchemaManagerLib
    PUBLIC EntitySchemaLib
    PRIVATE yaml-cpp FieldSchemaFactoryLib
)

target_include_directories(SchemaManagerLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =======================
# SchemaManager unit tests
# =======================
enable_testing()          # ensures tests can run

include(Catch)            # Catch2 CTest integration

add_executable(SchemaManagerTests
    tests/test_SchemaManager.cpp   # create this file in src/tests/
)

target_link_libraries(SchemaManagerTests
    PRIVATE FieldSchemaFactoryLib
    PRIVATE SchemaManagerLib
    PRIVATE yaml-cpp
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(SchemaManagerTests)
