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
            {
                throw std::runtime_error("Missing or invalid 'schemas'");
            }

            std::unordered_map<std::string, std::string> schemas;
            for (auto &el : request["schemas"].items())
            {
                schemas[el.key()] = el.value().get<std::string>();
            }

            return api.loadSchemas(schemas);
        }
        else if (command == "getSchemaList")
        {
            return api.getSchemaList();
        }

        else if (command == "loadData")
        {
            if (!request.contains("data") || !request["data"].is_object())
            {
                throw std::runtime_error("Missing or invalid 'data'");
            }

            std::unordered_map<std::string, std::string> data;
            for (auto &el : request["data"].items())
            {
                data[el.key()] = el.value().get<std::string>();
            }

            return api.loadData(data);
        }
        else if (command == "queryEntity")
        {
            return api.queryEntity(request.at("id").get<std::string>());
        }
        else if (command == "setField")
        {
            return api.setField(
                request.at("entityId").get<std::string>(),
                request.at("fieldName").get<std::string>(),
                request.at("value").get<std::string>());
        }
        else if (command == "validateEntity")
        {
            return api.validateEntity(request.at("entityId").get<std::string>());
        }
        else if (command == "getTree")
        {
            return api.getTree();
        }
        else if (command == "getSchema")
        {
            if (!request.contains("schemaName") || !request["schemaName"].is_string())
            {
                throw std::runtime_error("Missing or invalid 'schemaName'");
            }
            return api.getSchema(request["schemaName"].get<std::string>());
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
        return response.dump();
    }
}
