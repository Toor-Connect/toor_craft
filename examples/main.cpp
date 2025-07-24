#include <iostream>
#include <memory>
#include "Entity.h"
#include "EntitySchema.h"
#include "StringFieldSchema.h"
#include "IntegerFieldSchema.h"
#include "BooleanFieldSchema.h"
#include "StringFieldValue.h"
#include "IntegerFieldValue.h"
#include "BooleanFieldValue.h"
#include "FieldSchemaFactory.h"
#include "FieldValueFactory.h"

// You must have registered all schema and value types before this (some init function)

int main()
{
    registerFieldSchemaType<StringFieldSchema, StringFieldSchemaConfig>("string");
    registerFieldSchemaType<IntegerFieldSchema, IntegerFieldSchemaConfig>("integer");
    registerFieldSchemaType<BooleanFieldSchema, BooleanFieldSchemaConfig>("boolean");

    registerFieldValueType<StringFieldValue, StringFieldSchema>("string");
    registerFieldValueType<IntegerFieldValue, IntegerFieldSchema>("integer");
    registerFieldValueType<BooleanFieldValue, BooleanFieldSchema>("boolean");

    // Create configs
    StringFieldSchemaConfig nameConfig;
    nameConfig.name = "name";
    nameConfig.required = true;

    IntegerFieldSchemaConfig ageConfig;
    ageConfig.name = "age";
    ageConfig.required = false;
    ageConfig.minValue = 0;
    ageConfig.maxValue = 150;

    BooleanFieldSchemaConfig activeConfig;
    activeConfig.name = "active";
    activeConfig.required = false;

    // Then create schemas using factory
    auto nameFieldSchema = FieldSchemaFactory::instance().create("string", nameConfig);
    auto ageFieldSchema = FieldSchemaFactory::instance().create("integer", ageConfig);
    auto activeFieldSchema = FieldSchemaFactory::instance().create("boolean", activeConfig);

    // Create an EntitySchema and add the created field schemas
    EntitySchema personSchema("Person");
    personSchema.addField(std::move(nameFieldSchema));
    personSchema.addField(std::move(ageFieldSchema));
    personSchema.addField(std::move(activeFieldSchema));

    // Create the Entity
    Entity person(personSchema);

    std::string error;

    // Set field values via Entity API
    if (!person.setFieldValue("name", "Alice", error))
    {
        std::cerr << "Error setting name: " << error << std::endl;
    }
    if (!person.setFieldValue("age", "29", error))
    {
        std::cerr << "Error setting age: " << error << std::endl;
    }
    if (!person.setFieldValue("active", "true", error))
    {
        std::cerr << "Error setting active: " << error << std::endl;
    }

    // Validate the entity
    if (!person.validate(error))
    {
        std::cerr << "Validation failed: " << error << std::endl;
    }
    else
    {
        std::cout << "Entity is valid!" << std::endl;
    }

    // Output values
    auto nameValue = person.getFieldValue("name");
    if (nameValue)
        std::cout << "Name: " << nameValue->toString() << std::endl;

    auto ageValue = person.getFieldValue("age");
    if (ageValue)
        std::cout << "Age: " << ageValue->toString() << std::endl;

    auto activeValue = person.getFieldValue("active");
    if (activeValue)
        std::cout << "Active: " << activeValue->toString() << std::endl;

    return 0;
}
