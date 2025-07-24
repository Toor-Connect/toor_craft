# Define source files for Field library
set(FIELD_SOURCES
    StringField.cpp
    EnumField.cpp
    BooleanField.cpp
    FloatField.cpp
    IntegerField.cpp
)

add_library(FieldLib STATIC ${FIELD_SOURCES})

target_include_directories(FieldLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)