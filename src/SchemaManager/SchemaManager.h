#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class EntitySchema;

class SchemaManager
{
public:
    static SchemaManager &instance();

    void parseSchemaBundle(const std::unordered_map<std::string, std::string> &schemaContent);

    // 🔹 Profiles
    EntitySchema *getProfile(const std::string &profileName) const;
    std::vector<std::string> getProfileNames() const;
    bool profileExists(const std::string &profileName) const;

    // 🔹 Flat access to ALL entities (including profiles)
    std::vector<std::string> getEntityNames() const;
    EntitySchema *getEntity(const std::string &entityName) const;

    // 🔹 Maintenance
    void clear();

    const std::vector<std::unique_ptr<EntitySchema>> &getAllEntities() const { return entities_; }

private:
    SchemaManager();
    ~SchemaManager();
    SchemaManager(const SchemaManager &) = delete;
    SchemaManager &operator=(const SchemaManager &) = delete;

    // --- Storage ---
    std::vector<std::unique_ptr<EntitySchema>> entities_;          // owns ALL EntitySchemas
    std::unordered_map<std::string, EntitySchema *> profiles_;     // profile_name → root schema pointer
    std::unordered_map<std::string, EntitySchema *> entityLookup_; // entity_name → entity pointer (ALL entities)
};
