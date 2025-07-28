#include "SchemaManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>
#include <unordered_set>
#include <optional>
// TO DO! Create a command factory
#include "LuaCommand.h"
#include "LuaManager.h"

static std::unique_ptr<FieldSchema> buildFieldFromNode(const YAML::Node &fieldNode);
static std::unique_ptr<FieldSchema> buildPrimitiveField(const std::string &type, const YAML::Node &fieldNode, const std::string &name);
static std::unique_ptr<FieldSchema> buildObjectField(const YAML::Node &fieldNode, const std::string &name);
static std::unique_ptr<FieldSchema> buildArrayField(const YAML::Node &fieldNode, const std::string &name);
static void parseCommands(EntitySchema *entity, const YAML::Node &commandsNode);

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

EntitySchema *SchemaManager::getProfileSchema(const std::string &profileName) const
{
    auto it = profiles_.find(profileName);
    if (it == profiles_.end())
        throw std::runtime_error("Profile not found: " + profileName);
    return it->second;
}

std::vector<std::string> SchemaManager::getProfileSchemaNames() const
{
    std::vector<std::string> names;
    names.reserve(profiles_.size());
    for (const auto &pair : profiles_)
        names.push_back(pair.first);
    return names;
}

std::vector<std::string> SchemaManager::getEntitySchemaNames() const
{
    std::vector<std::string> names;
    names.reserve(entityLookup_.size());
    for (const auto &pair : entityLookup_)
        names.push_back(pair.first);
    return names;
}

EntitySchema *SchemaManager::getEntitySchema(const std::string &entityName) const
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

    return std::move(config);
}

static void parseCommands(EntitySchema *entity, const YAML::Node &commandsNode)
{
    for (auto it = commandsNode.begin(); it != commandsNode.end(); ++it)
    {
        std::string cmdName = it->first.as<std::string>();
        YAML::Node cmdNode = it->second;

        if (!cmdNode["file"])
        {
            throw std::runtime_error("Command '" + cmdName + "' is missing required 'file' key.");
        }

        std::string scriptFile = cmdNode["file"].as<std::string>();

        std::unordered_map<std::string, std::string> params;
        if (cmdNode["params"])
        {
            if (!cmdNode["params"].IsMap())
            {
                throw std::runtime_error("Command '" + cmdName + "' has 'params' but it's not a map.");
            }
            for (auto pit = cmdNode["params"].begin(); pit != cmdNode["params"].end(); ++pit)
            {
                params[pit->first.as<std::string>()] = pit->second.as<std::string>();
            }
        }

        LuaCommandConfig config;
        config.id = cmdName;
        config.type = "lua"; // we mark all as lua for now
        config.scriptPath = scriptFile;
        config.params = params;
        auto luaCmd = std::make_unique<LuaCommand>(std::move(config));
        entity->addCommand(std::move(luaCmd));
    }
}

static std::unique_ptr<FieldSchema> buildPrimitiveField(const std::string &type, const YAML::Node &fieldNode, const std::string &name)
{
    if (type == "boolean")
    {
        auto config = buildConfig<BooleanFieldSchemaConfig>(fieldNode, name);
        return FieldSchemaFactory::instance().create(type, std::move(config));
    }
    if (type == "string")
    {
        auto config = buildConfig<StringFieldSchemaConfig>(fieldNode, name);
        return FieldSchemaFactory::instance().create(type, std::move(config));
    }
    if (type == "integer")
    {
        auto config = buildConfig<IntegerFieldSchemaConfig>(fieldNode, name);
        config.minValue = fieldNode["min"] ? std::make_optional(fieldNode["min"].as<int64_t>()) : std::nullopt;
        config.maxValue = fieldNode["max"] ? std::make_optional(fieldNode["max"].as<int64_t>()) : std::nullopt;
        return FieldSchemaFactory::instance().create(type, std::move(config));
    }
    if (type == "float")
    {
        auto config = buildConfig<FloatFieldSchemaConfig>(fieldNode, name);
        config.minValue = fieldNode["min"] ? std::make_optional(fieldNode["min"].as<double>()) : std::nullopt;
        config.maxValue = fieldNode["max"] ? std::make_optional(fieldNode["max"].as<double>()) : std::nullopt;
        return FieldSchemaFactory::instance().create(type, std::move(config));
    }
    if (type == "enum")
    {
        auto config = buildConfig<EnumFieldSchemaConfig>(fieldNode, name);
        if (!fieldNode["values"] || !fieldNode["values"].IsSequence())
        {
            throw std::runtime_error("Enum field must have a 'values' array.");
        }
        for (const auto &valNode : fieldNode["values"])
        {
            config.allowedValues.push_back(valNode.as<std::string>());
        }
        return FieldSchemaFactory::instance().create(type, std::move(config));
    }
    if (type == "reference")
    {
        auto config = buildConfig<ReferenceFieldSchemaConfig>(fieldNode, name);
        if (fieldNode["target"])
        {
            config.targetEntityName = fieldNode["target"].as<std::string>();
        }
        return FieldSchemaFactory::instance().create(type, std::move(config));
    }

    throw std::runtime_error("Unsupported primitive type: " + type);
}

static std::unique_ptr<FieldSchema> buildObjectField(const YAML::Node &fieldNode, const std::string &name)
{
    ObjectFieldSchemaConfig config = buildConfig<ObjectFieldSchemaConfig>(fieldNode, name);

    auto objSchema = std::unique_ptr<ObjectFieldSchema>(
        static_cast<ObjectFieldSchema *>(FieldSchemaFactory::instance()
                                             .create("object", std::move(config))
                                             .release()));

    if (!fieldNode["fields"] || !fieldNode["fields"].IsMap())
    {
        throw std::runtime_error("Object field must have a 'fields' map.");
    }

    for (auto it = fieldNode["fields"].begin(); it != fieldNode["fields"].end(); ++it)
    {
        std::string childName = it->first.as<std::string>();
        YAML::Node childNode = it->second;

        if (!childNode["name"])
        {
            childNode["name"] = childName;
        }

        auto childSchema = buildFieldFromNode(childNode);
        objSchema->addField(std::move(childSchema));
    }

    return objSchema;
}

static std::unique_ptr<FieldSchema> buildArrayField(const YAML::Node &fieldNode, const std::string &name)
{
    ArrayFieldSchemaConfig config = buildConfig<ArrayFieldSchemaConfig>(fieldNode, name);

    if (!fieldNode["element"] || !fieldNode["element"]["type"])
    {
        throw std::runtime_error("Array field '" + name + "' must define 'element' with a 'type'.");
    }

    YAML::Node elemNode = fieldNode["element"];
    std::string elemType = elemNode["type"].as<std::string>();

    std::unique_ptr<FieldSchema> elementSchema;
    if (elemType == "object")
    {
        elementSchema = buildObjectField(elemNode, name + "_elem");
    }
    else if (elemType == "array")
    {
        elementSchema = buildArrayField(elemNode, name + "_elem");
    }
    else
    {
        elementSchema = buildPrimitiveField(elemType, elemNode, name + "_elem");
    }

    config.elementSchema = std::move(elementSchema);

    return FieldSchemaFactory::instance().create("array", std::move(config));
}

static std::unique_ptr<FieldSchema> buildFieldFromNode(const YAML::Node &fieldNode)
{
    if (!fieldNode["type"])
    {
        throw std::runtime_error("Field is missing a 'type' key.");
    }

    std::string type = fieldNode["type"].as<std::string>();
    std::string name = fieldNode["name"] ? fieldNode["name"].as<std::string>() : "";

    if (type == "object")
    {
        return buildObjectField(fieldNode, name);
    }
    if (type == "array")
    {
        return buildArrayField(fieldNode, name);
    }

    return buildPrimitiveField(type, fieldNode, name);
}

void SchemaManager::parseSchemaBundle(const std::unordered_map<std::string, std::string> &schemaContent)
{
    clear();
    for (const auto &pair : schemaContent)
    {
        const std::string &fileName = pair.first;
        const std::string &yamlContent = pair.second;

        YAML::Node node = YAML::Load(yamlContent);

        if (!node.IsMap())
        {
            throw std::runtime_error("Invalid schema format for file: " + fileName);
        }

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

        if (node["fields"] && !node["fields"].IsMap())
        {
            throw std::runtime_error(
                "In file '" + fileName + "', 'fields' must be a YAML map.");
        }

        if (node["children"] && !node["children"].IsMap())
        {
            throw std::runtime_error(
                "In file '" + fileName + "', 'children' must be a YAML map.");
        }

        if (node["commands"] && !node["commands"].IsMap())
        {
            throw std::runtime_error(
                "In file '" + fileName + "', 'commands' must be a YAML map.");
        }

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

        if (entityLookup_.find(name) != entityLookup_.end())
        {
            throw std::runtime_error("Duplicate entity/profile name detected: " + name);
        }

        auto entity = std::make_unique<EntitySchema>(name);
        EntitySchema *entityPtr = entity.get();

        entities_.push_back(std::move(entity));
        entityLookup_[name] = entityPtr;

        if (isProfile)
        {
            profiles_[name] = entityPtr;
        }
    }

    for (const auto &pair : schemaContent)
    {
        const std::string &yamlContent = pair.second;
        YAML::Node node = YAML::Load(yamlContent);

        std::string name = node["profile_name"]
                               ? node["profile_name"].as<std::string>()
                               : node["entity_name"].as<std::string>();

        EntitySchema *entity = entityLookup_[name];

        if (node["fields"])
        {
            for (auto it = node["fields"].begin(); it != node["fields"].end(); ++it)
            {
                std::string fieldName = it->first.as<std::string>();
                YAML::Node fieldNode = it->second;

                if (!fieldNode["type"])
                    throw std::runtime_error("Field '" + fieldName + "' must define a 'type'.");

                fieldNode["name"] = fieldName;

                auto schema = buildFieldFromNode(fieldNode);
                entity->addField(std::move(schema));
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

                entity->addChildSchema(relationTag, childIt->second);
            }
        }

        if (node["commands"])
        {
            parseCommands(entity, node["commands"]);
        }
    }
}

void SchemaManager::setBasePath(const std::filesystem::path &basePath)
{
    LuaManager::instance().setBasePath(basePath);
}
