#include "ToorCraftJSON.h"
#include "ToorCraftEngine.h"
#include "Entity.h"
#include "FieldValue.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ToorCraftJSON::ToorCraftJSON() : engine_(ToorCraftEngine::instance()) {}

ToorCraftJSON &ToorCraftJSON::instance()
{
    static ToorCraftJSON api;
    return api;
}

std::string ToorCraftJSON::loadSchemas(const std::unordered_map<std::string, std::string> &schemas)
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

std::string ToorCraftJSON::getSchemaList()
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

std::string ToorCraftJSON::loadData(const std::unordered_map<std::string, std::string> &data)
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

std::string ToorCraftJSON::queryEntity(const std::string &id)
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
        result["entity"] = json::parse(entity->getJson());
    }
    catch (const std::exception &e)
    {
        result["status"] = "error";
        result["message"] = e.what();
    }
    return result.dump(2);
}

std::string ToorCraftJSON::setField(const std::string &entityId, const std::string &fieldName, const std::string &value)
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

std::string ToorCraftJSON::validateEntity(const std::string &entityId)
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

std::string ToorCraftJSON::getSchema(const std::string &schemaName)
{
    nlohmann::json response;

    try
    {
        const EntitySchema *schema = engine_.getSchema(schemaName);
        if (!schema)
        {
            throw std::runtime_error("Schema not found: " + schemaName);
        }

        nlohmann::json schemaJson = nlohmann::json::parse(schema->toJson());

        response["status"] = "ok";
        response["schema"] = schemaJson;
    }
    catch (const std::exception &ex)
    {
        response["status"] = "error";
        response["message"] = ex.what();
    }

    return response.dump(2);
}

std::string ToorCraftJSON::getTree()
{
    json response;
    try
    {
        auto parents = engine_.getParents();

        json tree = json::array();
        for (auto *parent : parents)
        {
            json node;
            node["id"] = parent->getId();
            node["schema"] = parent->getSchema().getName();

            std::function<json(const Entity *)> collect;
            collect = [&](const Entity *entity) -> json
            {
                json children = json::array();
                auto *kids = engine_.getChildren(entity->getId());
                if (kids)
                {
                    for (auto *child : *kids)
                    {
                        json childNode;
                        childNode["id"] = child->getId();
                        childNode["schema"] = child->getSchema().getName();
                        childNode["children"] = collect(child); // recursion
                        children.push_back(childNode);
                    }
                }
                return children;
            };

            node["children"] = collect(parent);
            tree.push_back(node);
        }

        response["status"] = "ok";
        response["tree"] = tree;
    }
    catch (const std::exception &ex)
    {
        response["status"] = "error";
        response["message"] = ex.what();
    }
    return response.dump(2);
}

std::string ToorCraftJSON::getRoot()
{
    json response;
    try
    {
        auto parents = engine_.getParents();
        json rootArray = json::array();

        for (auto *parent : parents)
        {
            rootArray.push_back({{"id", parent->getId()},
                                 {"schema", parent->getSchema().getName()}});
        }

        response["status"] = "ok";
        response["root"] = rootArray;
    }
    catch (const std::exception &ex)
    {
        response["status"] = "error";
        response["message"] = ex.what();
    }
    return response.dump(2);
}

std::string ToorCraftJSON::getChildren(const std::string &entityId)
{
    json response;
    try
    {
        auto *children = engine_.getChildren(entityId);

        if (!children)
        {
            throw std::runtime_error("Entity '" + entityId + "' has no children or does not exist");
        }

        json childrenArray = json::array();
        for (auto *child : *children)
        {
            childrenArray.push_back({{"id", child->getId()},
                                     {"schema", child->getSchema().getName()}});
        }

        response["status"] = "ok";
        response["id"] = entityId;
        response["children"] = childrenArray;
    }
    catch (const std::exception &ex)
    {
        response["status"] = "error";
        response["message"] = ex.what();
    }
    return response.dump(2);
}

std::string ToorCraftJSON::getParent(const std::string &entityId)
{
    nlohmann::json response;
    try
    {
        Entity *entity = engine_.queryEntity(entityId);
        if (!entity)
        {
            throw std::runtime_error("Entity not found: " + entityId);
        }

        const std::string &parentId = entity->getParentId();
        if (parentId.empty())
        {
            response["status"] = "ok";
            response["parent"] = nullptr; // Explicit null for root entities
        }
        else
        {
            Entity *parent = engine_.queryEntity(parentId);
            if (!parent)
            {
                throw std::runtime_error("Parent entity not found: " + parentId);
            }

            response["status"] = "ok";
            response["parent"] = {
                {"id", parent->getId()},
                {"schema", parent->getSchema().getName()}};
        }
    }
    catch (const std::exception &ex)
    {
        response["status"] = "error";
        response["message"] = ex.what();
    }
    return response.dump(2);
}

std::string ToorCraftJSON::createEntity(const std::string &schemaName,
                                        const std::string &id,
                                        const std::string &parentId,
                                        const std::unordered_map<std::string, std::string> &fieldValues)
{
    nlohmann::json response;
    try
    {
        // delegate the heavy lifting to engine_
        engine_.createEntity(schemaName, id, parentId, fieldValues);

        response["status"] = "ok";
        response["created"] = {
            {"id", id},
            {"schema", schemaName},
            {"parentId", parentId.empty()
                             ? nlohmann::json(nullptr)
                             : nlohmann::json(parentId)}};
    }
    catch (const std::exception &ex)
    {
        response["status"] = "error";
        response["message"] = ex.what();
    }
    return response.dump(2);
}
