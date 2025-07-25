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
#include "LuaManager.h"
#include <fstream>
#include <cstdio> // for std::remove

class FindByIdQuery : public IEntityQuery
{
    std::string id_;

public:
    explicit FindByIdQuery(std::string id) : id_(std::move(id)) {}

    std::vector<Entity *> execute(const EntityManager &manager) const override
    {
        Entity *e = manager.getEntityById(id_);
        if (e)
            return {e};
        return {};
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <lua_base_directory>" << std::endl;
        return 1;
    }
    std::string luaBaseDir = argv[1];

    LuaManager::instance().setBaseDirectory(luaBaseDir);

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

    // Query for Alice by ID
    FindByIdQuery query("1");
    auto results = EntityManager::instance().query(query);
    if (!results.empty())
    {
        std::cout << "Found entity with ID 1: " << results[0]->getSchema().getName() << std::endl;
        auto fieldValue = results[0]->getFieldValue("name");
        if (fieldValue)
            std::cout << "Name: " << fieldValue->toString() << std::endl;
    }
    else
    {
        std::cout << "Entity with ID 1 not found." << std::endl;
    }
    const char *luaScriptPath = "validate.lua";

   const char *luaScriptContent = R"(
    -- Lua script expects two arguments: entityId, params (table)
    local entityId = ...
    local params = select(2, ...)

    local name = getField(entityId, "name")
    local age = getField(entityId, "age")

    if not name or name == "" then
        return false, "Name cannot be empty"
    end

    local minAge = tonumber(params["minAge"]) or 0
    local maxAge = tonumber(params["maxAge"]) or 150

    if age then
        -- Use C++ regexMatch function to check age is digits only
        local isNumber = regexMatch("^\\d+$", age)
        if not isNumber then
            return false, "Age must be a number: " .. age
        end

        local ageNum = tonumber(age)
        if not ageNum or ageNum < minAge or ageNum > maxAge then
            return false, string.format("Age must be between %d and %d", minAge, maxAge)
        end
    end

    -- Create a table with info
    local outputTable = {
        entityId = entityId,
        name = name,
        age = age or "N/A"
    }

    -- Print table contents
    for k, v in pairs(outputTable) do
        print(string.format("%s = %s", k, v))
    end

    local output = string.format("Entity %s: Name=%s, Age=%s\n", entityId, name, age or "N/A")
    local success, err = writeFile("output.txt", output)
    if not success then
        return false, "Failed to write output: " .. err
    end

    return true, ""
)";

    // Write the Lua script to file
    {
        std::ofstream luaFile(luaScriptPath);
        if (!luaFile)
        {
            std::cerr << "Failed to create Lua script file." << std::endl;
            return 1;
        }
        luaFile << luaScriptContent;
    }

    // Run Lua validation on Alice
    std::string luaError;
    Entity *aliceLua = EntityManager::instance().getEntityById("1");
    if (!aliceLua)
    {
        std::cerr << "Alice entity not found." << std::endl;
        return 1;
    }

    std::unordered_map<std::string, std::string> luaParams = {
        {"minAge", "0"},
        {"maxAge", "150"}};

    bool luaResult = LuaManager::instance().runScript(luaScriptPath, *aliceLua, luaParams, luaError);

    if (!luaResult)
    {
        std::cerr << "Lua validation failed: " << luaError << std::endl;
    }
    else
    {
        std::cout << "Lua validation succeeded." << std::endl;
    }

    // Delete the temporary Lua script file
    if (std::remove(luaScriptPath) != 0)
    {
        std::cerr << "Warning: Could not delete Lua script file." << std::endl;
    }

    return 0;
}
