# Define source files for Field library
set(FIELD_SOURCES
    FieldSchemaFactory.cpp
)

add_library(FieldSchemaFactoryLib STATIC ${FIELD_SOURCES})

target_link_libraries(FieldSchemaFactoryLib PUBLIC FieldSchemaLib)

target_include_directories(FieldSchemaFactoryLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)