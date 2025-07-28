#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <filesystem>

class EntitySchema;

class SchemaManager
{
public:
    static SchemaManager &instance();
    void setBasePath(const std::filesystem::path &basePath);
    void parseSchemaBundle(const std::unordered_map<std::string, std::string> &schemaContent);
    EntitySchema *getProfile(const std::string &profileName) const;
    std::vector<std::string> getProfileNames() const;
    bool profileExists(const std::string &profileName) const;
    std::vector<std::string> getEntityNames() const;
    EntitySchema *getEntity(const std::string &entityName) const;
    void clear();
    const std::vector<std::unique_ptr<EntitySchema>> &getAllEntities() const { return entities_; }

private:
    SchemaManager();
    ~SchemaManager();
    SchemaManager(const SchemaManager &) = delete;
    SchemaManager &operator=(const SchemaManager &) = delete;
    std::vector<std::unique_ptr<EntitySchema>> entities_;
    std::unordered_map<std::string, EntitySchema *> profiles_;
    std::unordered_map<std::string, EntitySchema *> entityLookup_;
};
