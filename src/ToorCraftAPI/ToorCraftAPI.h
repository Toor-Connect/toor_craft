#pragma once
#include <string>
#include <unordered_map>

class ToorCraftEngine;

class ToorCraftAPI
{
public:
    static ToorCraftAPI &instance();

    std::string loadSchemas(const std::unordered_map<std::string, std::string> &schemas);
    std::string getSchemaList();

    std::string loadData(const std::unordered_map<std::string, std::string> &data);
    std::string queryEntity(const std::string &id);
    std::string setField(const std::string &entityId, const std::string &fieldName, const std::string &value);
    std::string validateEntity(const std::string &entityId);

    std::string getTree();

private:
    ToorCraftAPI();
    ToorCraftAPI(const ToorCraftAPI &) = delete;
    ToorCraftAPI &operator=(const ToorCraftAPI &) = delete;

    ToorCraftEngine &engine_;
};
