#include <iostream>
#include <memory>
#include "Entity.h"
#include "EntitySchema.h"
#include "StringFieldSchema.h"
#include "IntegerFieldSchema.h"
#include "BooleanFieldSchema.h"
#include "ReferenceFieldSchema.h" // Add ReferenceFieldSchema
#include "StringFieldValue.h"
#include "IntegerFieldValue.h"
#include "BooleanFieldValue.h"
#include "ReferenceFieldValue.h" // Add ReferenceFieldValue
#include "FieldSchemaFactory.h"
#include "FieldValueFactory.h"
#include "EntityManager.h" // Add EntityManager

int main()
{
    // Register all schema and value types
    registerFieldSchemaType<StringFieldSchema, StringFieldSchemaConfig>("string");
    registerFieldSchemaType<IntegerFieldSchema, IntegerFieldSchemaConfig>("integer");
    registerFieldSchemaType<BooleanFieldSchema, BooleanFieldSchemaConfig>("boolean");
    registerFieldSchemaType<ReferenceFieldSchema, ReferenceFieldSchemaConfig>("reference");

    registerFieldValueType<StringFieldValue, StringFieldSchema>("string");
    registerFieldValueType<IntegerFieldValue, IntegerFieldSchema>("integer");
    registerFieldValueType<BooleanFieldValue, BooleanFieldSchema>("boolean");
    registerFieldValueType<ReferenceFieldValue, ReferenceFieldSchema>("reference");

    // Create configs for person schema fields
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

    // Create configs for a reference field
    ReferenceFieldSchemaConfig managerConfig;
    managerConfig.name = "managerId";
    managerConfig.required = false;
    managerConfig.referencedEntityType = "Person"; // Reference to another person

    // Create schemas using factory
    auto nameFieldSchema = FieldSchemaFactory::instance().create("string", nameConfig);
    auto ageFieldSchema = FieldSchemaFactory::instance().create("integer", ageConfig);
    auto activeFieldSchema = FieldSchemaFactory::instance().create("boolean", activeConfig);
    auto managerFieldSchema = FieldSchemaFactory::instance().create("reference", managerConfig);

    // Create an EntitySchema and add field schemas
    EntitySchema personSchema("Person");
    personSchema.addField(std::move(nameFieldSchema));
    personSchema.addField(std::move(ageFieldSchema));
    personSchema.addField(std::move(activeFieldSchema));
    personSchema.addField(std::move(managerFieldSchema));

    auto alice = std::make_unique<Entity>(personSchema);
    alice->setId("1");

    auto bob = std::make_unique<Entity>(personSchema);
    bob->setId("2");

    EntityManager::instance().addEntity(std::move(alice));
    EntityManager::instance().addEntity(std::move(bob));
    std::string error;

    // Set field values via EntityManager by entity ID
    if (!EntityManager::instance().setFieldValue("1", "name", "Alice", error))
        std::cerr << "Error setting Alice's name: " << error << std::endl;
    if (!EntityManager::instance().setFieldValue("1", "age", "29", error))
        std::cerr << "Error setting Alice's age: " << error << std::endl;
    if (!EntityManager::instance().setFieldValue("1", "active", "true", error))
        std::cerr << "Error setting Alice's active: " << error << std::endl;

    if (!EntityManager::instance().setFieldValue("2", "name", "Bob", error))
        std::cerr << "Error setting Bob's name: " << error << std::endl;
    if (!EntityManager::instance().setFieldValue("2", "age", "45", error))
        std::cerr << "Error setting Bob's age: " << error << std::endl;
    if (!EntityManager::instance().setFieldValue("2", "active", "true", error))
        std::cerr << "Error setting Bob's active: " << error << std::endl;

    // Bob is Alice's manager (reference field)
    if (!EntityManager::instance().setFieldValue("1", "managerId", "2", error))
        std::cerr << "Error setting Alice's managerId: " << error << std::endl;

    // Validate Alice entity
    if (!EntityManager::instance().validateEntity("1", error))
    {
        std::cerr << "Validation failed: " << error << std::endl;
    }
    else
    {
        std::cout << "Alice entity is valid!" << std::endl;
    }

    // Output Alice's info and manager reference
    auto nameValue = EntityManager::instance().getFieldValue("1", "name");
    if (nameValue)
        std::cout << "Name: " << nameValue->toString() << std::endl;

    auto managerValue = EntityManager::instance().getFieldValue("1", "managerId");
    if (managerValue)
        std::cout << "Manager ID: " << managerValue->toString() << std::endl;

    return 0;
}
