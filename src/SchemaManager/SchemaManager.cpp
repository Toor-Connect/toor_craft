#include "SchemaManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>
#include <unordered_set>
#include <optional>

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

template <typename ConfigType>
ConfigType buildConfig(const YAML::Node &fieldNode, const std::string &name)
{
    ConfigType config;
    config.name = name;
    config.required = fieldNode["required"] ? fieldNode["required"].as<bool>() : false;
    config.alias = fieldNode["alias"]
                       ? std::make_optional(fieldNode["alias"].as<std::string>())
                       : std::make_optional(name);

    return config;
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

        // ✅ NEW: Check top-level keys are valid
        static const std::unordered_set<std::string> validKeys = {
            "profile_name", "entity_name", "fields", "children", "commands"};

        for (const auto &kv : node)
        {
            std::string key = kv.first.as<std::string>();
            if (validKeys.find(key) == validKeys.end())
            {
                throw std::runtime_error(
                    "Invalid key '" + key + "' in schema file '" + fileName + "'");
            }
        }

        // ✅ NEW: Check "fields" must be a sequence if present
        if (node["fields"] && !node["fields"].IsSequence())
        {
            throw std::runtime_error(
                "In file '" + fileName + "', 'fields' must be a YAML sequence.");
        }

        // ✅ NEW: Check "children" must be a map if present
        if (node["children"] && !node["children"].IsMap())
        {
            throw std::runtime_error(
                "In file '" + fileName + "', 'children' must be a YAML map.");
        }

        // --- EXISTING CODE ---
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

    // --- PASS 2: Fill each entity (unchanged) ---
    for (const auto &pair : schemaContent)
    {
        const std::string &yamlContent = pair.second;
        YAML::Node node = YAML::Load(yamlContent);

        std::string name = node["profile_name"]
                               ? node["profile_name"].as<std::string>()
                               : node["entity_name"].as<std::string>();

        EntitySchema *entity = entityLookup_[name];

        // ✅ Your existing deep validation code for fields, enums, references, etc.
        // (kept as-is from your previous implementation)
        if (node["fields"])
        {
            for (const auto &fieldNode : node["fields"])
            {
                if (!fieldNode.IsMap())
                    throw std::runtime_error("Check the format in schema " + name);

                if (!fieldNode["name"] || !fieldNode["type"])
                    throw std::runtime_error("Check the format of fields in schema " + name);

                std::string name = fieldNode["name"].as<std::string>();
                std::string type = fieldNode["type"].as<std::string>();

                static const std::unordered_set<std::string> validTypes = {
                    "boolean", "string", "integer", "float", "enum", "reference"};

                if (validTypes.find(type) == validTypes.end())
                {
                    std::string validTypesStr;
                    for (const auto &t : validTypes)
                    {
                        if (!validTypesStr.empty())
                            validTypesStr += ", ";
                        validTypesStr += t;
                    }
                    throw std::runtime_error(
                        "Invalid field type '" + type + "' in schema " + name +
                        ". Valid types are: " + validTypesStr);
                }

                std::unique_ptr<FieldSchema> fieldSchema;

                if (type == "boolean")
                {
                    auto config = buildConfig<BooleanFieldSchemaConfig>(fieldNode, name);
                    fieldSchema = FieldSchemaFactory::instance().create(type, config);
                }
                else if (type == "string")
                {
                    auto config = buildConfig<StringFieldSchemaConfig>(fieldNode, name);
                    fieldSchema = FieldSchemaFactory::instance().create(type, config);
                }
                else if (type == "integer")
                {
                    auto config = buildConfig<IntegerFieldSchemaConfig>(fieldNode, name);
                    config.minValue = fieldNode["min"]
                                          ? std::make_optional(fieldNode["min"].as<int64_t>())
                                          : std::nullopt;
                    config.maxValue = fieldNode["max"]
                                          ? std::make_optional(fieldNode["max"].as<int64_t>())
                                          : std::nullopt;
                    fieldSchema = FieldSchemaFactory::instance().create(type, config);
                }
                else if (type == "float")
                {
                    auto config = buildConfig<FloatFieldSchemaConfig>(fieldNode, name);
                    config.minValue = fieldNode["min"]
                                          ? std::make_optional(fieldNode["min"].as<double>())
                                          : std::nullopt;
                    config.maxValue = fieldNode["max"]
                                          ? std::make_optional(fieldNode["max"].as<double>())
                                          : std::nullopt;
                    fieldSchema = FieldSchemaFactory::instance().create(type, config);
                }
                else if (type == "enum")
                {
                    EnumFieldSchemaConfig config = buildConfig<EnumFieldSchemaConfig>(fieldNode, name);

                    if (!fieldNode["values"] || !fieldNode["values"].IsSequence())
                    {
                        throw std::runtime_error(
                            "Enum field '" + name + "' in schema '" + entity->getName() +
                            "' must have a YAML sequence 'values'.");
                    }

                    for (const auto &valueNode : fieldNode["values"])
                    {
                        if (!valueNode.IsScalar())
                        {
                            throw std::runtime_error(
                                "Enum field '" + name + "' in schema '" + entity->getName() +
                                "' has an invalid entry in 'values'. Each value must be a string.");
                        }

                        std::string value = valueNode.as<std::string>();
                        if (value.empty())
                        {
                            throw std::runtime_error(
                                "Enum field '" + name + "' in schema '" + entity->getName() +
                                "' has an empty string in 'values'.");
                        }

                        config.allowedValues.push_back(value);
                    }

                    if (config.allowedValues.empty())
                    {
                        throw std::runtime_error(
                            "Enum field '" + name + "' in schema '" + entity->getName() +
                            "' must have at least one value in 'values'.");
                    }

                    fieldSchema = FieldSchemaFactory::instance().create(type, config);
                }
                else if (type == "reference")
                {
                    ReferenceFieldSchemaConfig config = buildConfig<ReferenceFieldSchemaConfig>(fieldNode, name);
                    if (fieldNode["target"])
                    {
                        config.targetEntityName = fieldNode["target"].as<std::string>();
                        if (entityLookup_.find(config.targetEntityName) == entityLookup_.end())
                        {
                            throw std::runtime_error(
                                "Reference field '" + name + "' in schema '" + entity->getName() +
                                "' points to non-existent entity '" + config.targetEntityName + "'.");
                        }
                    }
                    fieldSchema = FieldSchemaFactory::instance().create(type, config);
                }

                if (fieldSchema)
                {
                    entity->addField(std::move(fieldSchema));
                }
            }
        }

        if (node["children"])
        {
            for (auto it = node["children"].begin(); it != node["children"].end(); ++it)
            {
                std::string relationTag = it->first.as<std::string>();
                YAML::Node childNode = it->second;

                if (!childNode["entity"])
                {
                    throw std::runtime_error(
                        "Child relation '" + relationTag +
                        "' in schema '" + name + "' must specify 'entity'.");
                }

                std::string childEntityName = childNode["entity"].as<std::string>();

                auto childIt = entityLookup_.find(childEntityName);
                if (childIt == entityLookup_.end())
                {
                    throw std::runtime_error(
                        "Child entity '" + childEntityName +
                        "' referenced by relation '" + relationTag +
                        "' in schema '" + name + "' does not exist.");
                }

                // ✅ Now we store by relationTag, not by entity name
                entity->addChildSchema(relationTag, childIt->second);
            }
        }
    }
}
