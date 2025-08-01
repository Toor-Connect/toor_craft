#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Entity;
class EntitySchema;

class ToorCraftEngine
{
public:
    static ToorCraftEngine &instance();

    void loadSchemas(const std::unordered_map<std::string, std::string> &schemas);
    std::vector<std::string> getSchemaList() const;
    const EntitySchema *getSchema(const std::string &name) const;

    void loadData(const std::unordered_map<std::string, std::string> &data);
    Entity *queryEntity(const std::string &id) const;
    void setField(const std::string &entityId, const std::string &fieldName, const std::string &value);
    void validateEntity(const std::string &entityId);

    std::vector<Entity *> getParents() const;
    const std::vector<Entity *> *getChildren(const std::string &parentId) const;
    std::string getParent(const std::string &entityId) const;
    void createEntity(const std::string &schemaName,
                      const std::string &entityId,
                      const std::string &parentId,
                      const std::unordered_map<std::string, std::string> &fieldData);

private:
    ToorCraftEngine() = default;
    ToorCraftEngine(const ToorCraftEngine &) = delete;
    ToorCraftEngine &operator=(const ToorCraftEngine &) = delete;
};
