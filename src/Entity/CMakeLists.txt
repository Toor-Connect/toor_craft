# Define source files for Field library
set(SOURCES
    Entity.cpp
)

add_library(EntityLib STATIC ${SOURCES})

target_link_libraries(EntityLib PUBLIC EntitySchemaLib FieldValueLib)
target_link_libraries(EntityLib PRIVATE FieldValueFactoryLib)

target_include_directories(EntityLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)