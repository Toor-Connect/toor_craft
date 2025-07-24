# Define source files for Field library
set(FIELD_SOURCES
    BooleanFieldValue.cpp
    EnumFieldValue.cpp
    FloatFieldValue.cpp
    IntegerFieldValue.cpp
    StringFieldValue.cpp
)

add_library(FieldValuesLib STATIC ${FIELD_SOURCES})

target_link_libraries(FieldValuesLib PUBLIC FieldSchemaLib)

target_include_directories(FieldValuesLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)