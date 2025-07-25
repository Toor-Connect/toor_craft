# Define source files for Field library
set(SOURCES
    EntityManager.cpp
)

add_library(EntityManagerLib STATIC ${SOURCES})

target_link_libraries(EntityManagerLib PUBLIC EntityLib FieldValueLib)

target_include_directories(EntityManagerLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)