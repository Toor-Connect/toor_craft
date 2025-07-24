# Define source files for Field library
set(SOURCES
    FieldValueFactory.cpp
)

add_library(FieldValueFactoryLib STATIC ${SOURCES})

target_link_libraries(FieldValueFactoryLib PUBLIC FieldSchemaLib FieldValueLib)

target_include_directories(FieldValueFactoryLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)