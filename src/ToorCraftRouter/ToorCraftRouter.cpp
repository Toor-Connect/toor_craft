#include "ToorCraftRouter.h"
#include "ToorCraftJSON.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ToorCraftRouter &ToorCraftRouter::instance()
{
    static ToorCraftRouter inst;
    return inst;
}

std::string ToorCraftRouter::handleRequest(const std::string &jsonRequest)
{
    auto &api = ToorCraftJSON::instance();
    json response;

    try
    {
        json request = json::parse(jsonRequest);

        if (!request.contains("command") || !request["command"].is_string())
        {
            throw std::runtime_error("Missing or invalid 'command'");
        }

        std::string command = request["command"];

        if (command == "loadSchemas")
        {
            if (!request.contains("schemas") || !request["schemas"].is_object())
                throw std::runtime_error("Missing or invalid 'schemas'");

            std::unordered_map<std::string, std::string> schemas;
            for (auto &el : request["schemas"].items())
                schemas[el.key()] = el.value().get<std::string>();

            return api.loadSchemas(schemas);
        }
        else if (command == "getSchemaList")
        {
            return api.getSchemaList();
        }
        else if (command == "getSchema")
        {
            if (!request.contains("schema") || !request["schema"].is_string())
                throw std::runtime_error("Missing or invalid 'schemaName'");

            return api.getSchema(request["schema"].get<std::string>());
        }
        else if (command == "loadData")
        {
            if (!request.contains("data") || !request["data"].is_object())
                throw std::runtime_error("Missing or invalid 'data'");

            std::unordered_map<std::string, std::string> data;
            for (auto &el : request["data"].items())
                data[el.key()] = el.value().get<std::string>();

            return api.loadData(data);
        }
        else if (command == "queryEntity")
        {
            if (!request.contains("id") || !request["id"].is_string())
                throw std::runtime_error("Missing or invalid 'id'");

            return api.queryEntity(request["id"].get<std::string>());
        }
        else if (command == "setField")
        {
            for (auto key : {"id", "field", "value"})
            {
                if (!request.contains(key) || !request[key].is_string())
                    throw std::runtime_error(std::string("Missing or invalid '") + key + "'");
            }

            return api.setField(
                request["id"].get<std::string>(),
                request["field"].get<std::string>(),
                request["value"].get<std::string>());
        }
        else if (command == "validateEntity")
        {
            if (!request.contains("id") || !request["id"].is_string())
                throw std::runtime_error("Missing or invalid 'entityId'");

            return api.validateEntity(request["id"].get<std::string>());
        }
        else if (command == "getTree")
        {
            return api.getTree();
        }
        else if (command == "getRoot")
        {
            return api.getRoot();
        }
        else if (command == "getChildren")
        {
            if (!request.contains("parentId") || !request["parentId"].is_string())
                throw std::runtime_error("Missing or invalid 'parentId'");

            return api.getChildren(request["parentId"].get<std::string>());
        }
        else if (command == "createEntity")
        {
            if (!request.contains("schema") || !request["schema"].is_string())
                throw std::runtime_error("Missing or invalid 'schemaName'");
            if (!request.contains("id") || !request["id"].is_string())
                throw std::runtime_error("Missing or invalid 'entityId'");
            if (!request.contains("payload") || !request["payload"].is_object())
                throw std::runtime_error("Missing or invalid 'payload'");

            std::string schemaName = request["schema"].get<std::string>();
            std::string entityId = request["id"].get<std::string>();
            std::string parentId = request.contains("parentId") && request["parentId"].is_string()
                                       ? request["parentId"].get<std::string>()
                                       : "";

            std::unordered_map<std::string, std::string> payload;
            for (auto &el : request["payload"].items())
                payload[el.key()] = el.value().dump(); // 🔑 Serialize objects/arrays as strings

            return api.createEntity(schemaName, entityId, parentId, payload);
        }
        else if (command == "getParent")
        {
            if (!request.contains("id") || !request["id"].is_string())
                throw std::runtime_error("Missing or invalid 'entityId'");

            return api.getParent(request["id"].get<std::string>());
        }
        else if (command == "deleteEntity")
        {
            if (!request.contains("id") || !request["id"].is_string())
                throw std::runtime_error("Missing or invalid 'id'");

            return api.deleteEntity(request["id"].get<std::string>());
        }
        else
        {
            throw std::runtime_error("Unknown command: " + command);
        }
    }
    catch (const std::exception &e)
    {
        response["status"] = "error";
        response["message"] = e.what();
        return response.dump(2);
    }
}
