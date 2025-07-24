# Define source files for Field library
set(SOURCES
    BooleanFieldValue.cpp
    EnumFieldValue.cpp
    FloatFieldValue.cpp
    IntegerFieldValue.cpp
    StringFieldValue.cpp
)

add_library(FieldValueLib STATIC ${SOURCES})

target_link_libraries(FieldValueLib PUBLIC FieldSchemaLib)

target_include_directories(FieldValueLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)