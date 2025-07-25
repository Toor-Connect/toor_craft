# Define source files for Field library
set(SOURCES
    EntitySchema.cpp
)

add_library(EntitySchemaLib STATIC ${SOURCES})

target_link_libraries(EntitySchemaLib PUBLIC FieldSchemaLib CommandLib)

target_include_directories(EntitySchemaLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)