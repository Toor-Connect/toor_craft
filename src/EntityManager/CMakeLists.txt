# Define source files for Field library
set(SOURCES
    EntityManager.cpp
)

add_library(EntityManagerLib STATIC ${SOURCES})

target_link_libraries(EntityManagerLib PUBLIC EntityLib FieldValueLib)
target_link_libraries(EntityManagerLib PRIVATE SchemaManagerLib FieldValueFactoryLib yaml-cpp)

target_include_directories(EntityManagerLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_testing()          # ensures tests can run

include(Catch)            # Catch2 CTest integration

add_executable(EntityManagerTests
    tests/test_EntityManager.cpp   # create this file in src/tests/
)

target_link_libraries(EntityManagerTests
    PRIVATE EntityManagerLib
    PRIVATE SchemaManagerLib
    PRIVATE yaml-cpp
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(EntityManagerTests)