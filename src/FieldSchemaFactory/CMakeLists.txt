# Define source files for Field library
set(SOURCES
    FieldSchemaFactory.cpp
)

add_library(FieldSchemaFactoryLib STATIC ${SOURCES})

target_link_libraries(FieldSchemaFactoryLib PUBLIC FieldSchemaLib)

target_include_directories(FieldSchemaFactoryLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)