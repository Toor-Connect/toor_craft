#include "ToorCraftAPI.h"
#include "ToorCraftEngine.h"
#include "Entity.h"
#include "FieldValue.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ToorCraftAPI::ToorCraftAPI() : engine_(ToorCraftEngine::instance()) {}

ToorCraftAPI &ToorCraftAPI::instance()
{
    static ToorCraftAPI api;
    return api;
}

std::string ToorCraftAPI::loadSchemas(const std::unordered_map<std::string, std::string> &schemas)
{
    json result;
    try
    {
        engine_.loadSchemas(schemas);
        result["status"] = "ok";
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}

std::string ToorCraftAPI::getSchemaList()
{
    json result;
    try
    {
        auto schemas = engine_.getSchemaList();
        result["status"] = "ok";
        result["schemas"] = schemas;
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}

std::string ToorCraftAPI::loadData(const std::unordered_map<std::string, std::string> &data)
{
    json result;
    try
    {
        engine_.loadData(data);
        result["status"] = "ok";
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}

std::string ToorCraftAPI::queryEntity(const std::string &id)
{
    json result;
    try
    {
        Entity *entity = engine_.queryEntity(id);
        if (!entity)
        {
            result["status"] = "not_found";
            return result.dump();
        }

        result["status"] = "ok";
        result["entity"]["id"] = entity->getId();
        result["entity"]["schema"] = entity->getSchema().getName();

        auto dict = entity->getDict();
        for (const auto &pair : dict)
        {
            result["entity"][pair.first] = pair.second;
        }
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}

std::string ToorCraftAPI::setField(const std::string &entityId, const std::string &fieldName, const std::string &value)
{
    json result;
    try
    {
        engine_.setField(entityId, fieldName, value);
        result["status"] = "ok";
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}

std::string ToorCraftAPI::validateEntity(const std::string &entityId)
{
    json result;
    try
    {
        engine_.validateEntity(entityId);
        result["status"] = "ok";
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}

std::string ToorCraftAPI::getTree()
{
    json result;
    try
    {
        auto parents = engine_.getParents();
        result["status"] = "ok";
        json tree = json::array();

        for (auto *parent : parents)
        {
            json parentNode;
            parentNode["id"] = parent->getId();
            parentNode["schema"] = parent->getSchema().getName();
            parentNode["children"] = json::array();

            const auto *children = engine_.getChildren(parent->getId());
            if (children)
            {
                for (auto *child : *children)
                {
                    parentNode["children"].push_back(child->getId());
                }
            }
            tree.push_back(parentNode);
        }

        result["tree"] = tree;
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump();
}
