# Define source files for Field library
set(FIELD_SOURCES
    StringFieldSchema.cpp
    EnumFieldSchema.cpp
    BooleanFieldSchema.cpp
    FloatFieldSchema.cpp
    IntegerFieldSchema.cpp
)

add_library(FieldSchemaLib STATIC ${FIELD_SOURCES})

target_include_directories(FieldSchemaLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)