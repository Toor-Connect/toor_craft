#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <utility>
#include <unordered_map>
#include <cstdio>
#include "LuaManager.h"
#include "Entity.h"
#include "EntityManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"
#include "FieldValueFactory.h"
#include "StringFieldSchema.h"
#include "IntegerFieldSchema.h"
#include "BooleanFieldSchema.h"
#include "ReferenceFieldSchema.h"
#include "StringFieldValue.h"
#include "IntegerFieldValue.h"
#include "BooleanFieldValue.h"
#include "ReferenceFieldValue.h"

int main(int argc, char *argv[])
{
    // if (argc < 2)
    //{
    //     std::cerr << "Usage: " << argv[0] << " <lua_base_directory>" << std::endl;
    //     return 1;
    // }
    // std::string luaBaseDir = argv[1];
    // LuaManager::instance().setBaseDirectory(luaBaseDir);

    // Register all schema and value types

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

    ReferenceFieldSchemaConfig managerConfig;
    managerConfig.name = "managerId";
    managerConfig.required = false;
    managerConfig.referencedEntityType = "Person";

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

    // Create two people: Alice and Bob
    auto alice = std::make_unique<Entity>(personSchema);
    alice->setId("1");

    auto bob = std::make_unique<Entity>(personSchema);
    bob->setId("2");

    EntityManager::instance().addEntity(std::move(alice));
    EntityManager::instance().addEntity(std::move(bob));
    std::string error;

    // Set values
    EntityManager::instance().setFieldValue("1", "name", "Alice", error);
    EntityManager::instance().setFieldValue("1", "age", "29", error);
    EntityManager::instance().setFieldValue("1", "active", "true", error);

    EntityManager::instance().setFieldValue("2", "name", "Bob", error);
    EntityManager::instance().setFieldValue("2", "age", "45", error);
    EntityManager::instance().setFieldValue("2", "active", "true", error);

    // Bob is Alice's manager
    EntityManager::instance().setFieldValue("1", "managerId", "2", error);

    // Validate Alice entity
    if (!EntityManager::instance().validateEntity("1", error))
        std::cerr << "Validation failed: " << error << std::endl;
    else
        std::cout << "Alice entity is valid!" << std::endl;

    // Write Lua script file
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
            -- Validate age format using regexMatch
            local isNumber = regexMatch("^\\d+$", age)
            if not isNumber then
                return false, "Age must be a number: " .. age
            end

            local ageNum = tonumber(age)
            if not ageNum or ageNum < minAge or ageNum > maxAge then
                return false, string.format("Age must be between %d and %d", minAge, maxAge)
            end
        end

        -- Get entity dictionary as a Lua table (no JSON decode needed)
        local dict = getDict(entityId)
        print("Entity dict contents:")
        for k, v in pairs(dict) do
            print(k, v)
        end

        -- Write a summary file using writeFile API
        local output = string.format("Entity %s: Name=%s, Age=%s\n", entityId, name, age or "N/A")
        local success, err = writeFile("output.txt", output)
        if not success then
            return false, "Failed to write output: " .. err
        end

        return true, ""
    )";

    {
        std::ofstream luaFile(luaScriptPath);
        if (!luaFile)
        {
            std::cerr << "Failed to create Lua script file." << std::endl;
            return 1;
        }
        luaFile << luaScriptContent;
    }

    // Create a simple template file for inja to render
    {
        std::ofstream templateFile("template.txt");
        templateFile << "Hello, my name is {{ name }} and I am {{ age }} years old.";
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

    // Cleanup: remove script and template
    std::remove(luaScriptPath);
    std::remove("template.txt");

    return 0;
}
