add_executable(example_app main.cpp)

target_link_libraries(example_app PRIVATE 
EntitySchemaLib 
EntityLib 
FieldSchemaLib 
FieldSchemaFactoryLib 
FieldValueFactoryLib
EntityManagerLib
CommandLib
)