# Define source files for Field library
set(SOURCES
    StringFieldSchema.cpp
    EnumFieldSchema.cpp
    BooleanFieldSchema.cpp
    FloatFieldSchema.cpp
    IntegerFieldSchema.cpp
)

add_library(FieldSchemaLib STATIC ${SOURCES})

target_include_directories(FieldSchemaLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)