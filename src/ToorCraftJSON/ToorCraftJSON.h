#pragma once
#include <string>
#include <unordered_map>

class ToorCraftEngine;

class ToorCraftJSON
{
public:
    static ToorCraftJSON &instance();

    std::string loadSchemas(const std::unordered_map<std::string, std::string> &schemas);
    std::string getSchemaList();
    std::string getSchema(const std::string &schemaName);

    std::string loadData(const std::unordered_map<std::string, std::string> &data);
    std::string queryEntity(const std::string &id);
    std::string setField(const std::string &entityId, const std::string &fieldName, const std::string &value);
    std::string validateEntity(const std::string &entityId);

    std::string getTree();
    std::string getRoot();
    std::string getChildren(const std::string &entityId);

    std::string getParent(const std::string &entityId);
    std::string createEntity(const std::string &schemaName,
                             const std::string &id,
                             const std::string &parentId,
                             const std::unordered_map<std::string, std::string> &fieldValues);
    std::string deleteEntity(const std::string &entityId);

private:
    ToorCraftJSON();
    ToorCraftJSON(const ToorCraftJSON &) = delete;
    ToorCraftJSON &operator=(const ToorCraftJSON &) = delete;

    ToorCraftEngine &engine_;
};
