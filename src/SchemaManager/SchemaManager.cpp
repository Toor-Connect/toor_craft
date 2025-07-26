#include "SchemaManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>

SchemaManager &SchemaManager::instance()
{
    static SchemaManager instance;
    return instance;
}

SchemaManager::SchemaManager() = default;
SchemaManager::~SchemaManager() = default;

void SchemaManager::clear()
{
    profiles_.clear();
    entityLookup_.clear();
    entities_.clear();
}

bool SchemaManager::profileExists(const std::string &profileName) const
{
    return profiles_.find(profileName) != profiles_.end();
}

EntitySchema *SchemaManager::getProfile(const std::string &profileName) const
{
    auto it = profiles_.find(profileName);
    if (it == profiles_.end())
        throw std::runtime_error("Profile not found: " + profileName);
    return it->second;
}

std::vector<std::string> SchemaManager::getProfileNames() const
{
    std::vector<std::string> names;
    names.reserve(profiles_.size());
    for (const auto &pair : profiles_)
        names.push_back(pair.first);
    return names;
}

std::vector<std::string> SchemaManager::getEntityNames() const
{
    std::vector<std::string> names;
    names.reserve(entityLookup_.size());
    for (const auto &pair : entityLookup_)
        names.push_back(pair.first);
    return names;
}

EntitySchema *SchemaManager::getEntity(const std::string &entityName) const
{
    auto it = entityLookup_.find(entityName);
    if (it == entityLookup_.end())
        return nullptr;
    return it->second;
}

void SchemaManager::parseSchemaBundle(const std::unordered_map<std::string, std::string> &schemaContent)
{
    clear();

    // --- PASS 1: Validate and register all entities ---
    for (const auto &pair : schemaContent)
    {
        const std::string &fileName = pair.first;
        const std::string &yamlContent = pair.second;

        YAML::Node node = YAML::Load(yamlContent);

        if (!node.IsMap())
        {
            throw std::runtime_error("Invalid schema format for file: " + fileName);
        }

        // Must have either profile_name or entity_name
        std::string name;
        bool isProfile = false;
        if (node["profile_name"])
        {
            name = node["profile_name"].as<std::string>();
            isProfile = true;
        }
        else if (node["entity_name"])
        {
            name = node["entity_name"].as<std::string>();
        }
        else
        {
            throw std::runtime_error("File '" + fileName + "' must define either profile_name or entity_name.");
        }

        // Check for duplicates
        if (entityLookup_.find(name) != entityLookup_.end())
        {
            throw std::runtime_error("Duplicate entity/profile name detected: " + name);
        }

        // Create empty skeleton entity
        auto entity = std::make_unique<EntitySchema>(name);
        EntitySchema *entityPtr = entity.get();

        entities_.push_back(std::move(entity));
        entityLookup_[name] = entityPtr;

        if (isProfile)
        {
            profiles_[name] = entityPtr;
        }
    }

    // --- PASS 2: Fill each entity ---
    for (const auto &pair : schemaContent)
    {
        const std::string &yamlContent = pair.second;
        YAML::Node node = YAML::Load(yamlContent);

        std::string name = node["profile_name"]
                               ? node["profile_name"].as<std::string>()
                               : node["entity_name"].as<std::string>();

        EntitySchema *entity = entityLookup_[name];

        // if (node["fields"])
        //{
        //     for (const auto &fieldNode : node["fields"])
        //     {
        //         auto fieldSchema = FieldSchemaFactory::createFieldSchema(fieldNode);
        //         entity->addField(std::move(fieldSchema));
        //     }
        // }

        // if (node["commands"])
        //{
        //     for (const auto &commandNode : node["commands"])
        //     {
        //         auto command = CommandFactory::createCommand(commandNode);
        //         entity->addCommand(std::move(command));
        //     }
        // }

        // ✅ Link children (no recursion, just lookup by name)
        if (node["children"])
        {
            for (auto it = node["children"].begin(); it != node["children"].end(); ++it)
            {
                std::string childName = it->first.as<std::string>();

                auto childIt = entityLookup_.find(childName);
                if (childIt == entityLookup_.end())
                {
                    throw std::runtime_error(
                        "Child entity '" + childName +
                        "' referenced by '" + name + "' does not exist.");
                }

                entity->addChildSchema(childName, childIt->second);
            }
        }
    }
}
