#include "EntityManager.h"
#include "SchemaManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"
#include "FieldValueFactory.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>
#include <algorithm>

static void populateFieldValue(FieldValue *fieldValue, const FieldSchema &schema, const YAML::Node &node);
static void populateObjectField(ObjectFieldValue *objValue, const ObjectFieldSchema &objSchema, const YAML::Node &node);
static void populateArrayField(ArrayFieldValue *arrValue, const ArrayFieldSchema &arrSchema, const YAML::Node &node);

static void populateObjectField(ObjectFieldValue *objValue, const ObjectFieldSchema &objSchema, const YAML::Node &node)
{
    if (!node.IsMap())
        throw std::runtime_error("Expected YAML map for object field");

    for (auto it = node.begin(); it != node.end(); ++it)
    {
        std::string subKey = it->first.as<std::string>();
        YAML::Node subNode = it->second;

        const FieldSchema *subSchema = objSchema.getField(subKey);
        if (!subSchema)
            throw std::runtime_error("Subfield '" + subKey + "' not defined in object schema");

        FieldValue *subValue = objValue->getFieldValue(subKey);
        if (!subValue)
            throw std::runtime_error("Subfield '" + subKey + "' not present in object value");

        populateFieldValue(subValue, *subSchema, subNode);
    }
}

static void populateArrayField(ArrayFieldValue *arrValue, const ArrayFieldSchema &arrSchema, const YAML::Node &node)
{
    if (!node.IsSequence())
        throw std::runtime_error("Expected YAML sequence for array field");

    for (std::size_t i = 0; i < node.size(); ++i)
    {
        YAML::Node elemNode = node[i];

        auto elemValue = FieldValueFactory::instance().create(
            arrSchema.getElementSchema().getTypeName(),
            arrSchema.getElementSchema());

        populateFieldValue(elemValue.get(), arrSchema.getElementSchema(), elemNode);

        arrValue->addElement(std::move(elemValue));
    }
}

static void populateFieldValue(FieldValue *fieldValue, const FieldSchema &schema, const YAML::Node &node)
{
    const std::string &type = schema.getTypeName();

    if (type == "object")
    {
        auto *objVal = dynamic_cast<ObjectFieldValue *>(fieldValue);
        if (!objVal)
            throw std::runtime_error("Schema says object but value is not ObjectFieldValue");

        const ObjectFieldSchema &objSchema = static_cast<const ObjectFieldSchema &>(schema);
        populateObjectField(objVal, objSchema, node);
    }
    else if (type == "array")
    {
        auto *arrVal = dynamic_cast<ArrayFieldValue *>(fieldValue);
        if (!arrVal)
            throw std::runtime_error("Schema says array but value is not ArrayFieldValue");

        const ArrayFieldSchema &arrSchema = static_cast<const ArrayFieldSchema &>(schema);
        populateArrayField(arrVal, arrSchema, node);
    }
    else
    {
        if (!node.IsScalar())
            throw std::runtime_error("Expected scalar for primitive field type: " + type);

        fieldValue->setValueFromString(node.as<std::string>());
    }
}

EntityManager &EntityManager::instance()
{
    static EntityManager manager;
    return manager;
}

void EntityManager::addEntity(std::unique_ptr<Entity> entity)
{
    const std::string &id = entity->getId();
    const std::string &parentId = entity->getParentId();

    entities_.emplace(id, std::move(entity));

    Entity *ptr = entities_[id].get();
    if (!parentId.empty())
    {
        childrenIndex_[parentId].push_back(ptr);
    }
    else
    {
        parents_.push_back(ptr);
    }
}

Entity *EntityManager::getEntityById(const std::string &id) const
{
    auto it = entities_.find(id);
    if (it != entities_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

bool EntityManager::removeEntity(const std::string &id)
{
    auto it = entities_.find(id);
    if (it == entities_.end())
        return false;

    const std::string &parentId = it->second->getParentId();

    if (!parentId.empty())
    {
        auto childIt = childrenIndex_.find(parentId);
        if (childIt != childrenIndex_.end())
        {
            auto &siblings = childIt->second;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), it->second.get()), siblings.end());

            if (siblings.empty())
            {
                childrenIndex_.erase(childIt);
            }
        }
    }

    entities_.erase(it);
    return true;
}

void EntityManager::clear()
{
    entities_.clear();
    childrenIndex_.clear();
    parents_.clear();
}

void EntityManager::setFieldValue(const std::string &entityId,
                                  const std::string &fieldName,
                                  const std::string &value)
{
    auto entity = getEntityById(entityId);
    if (!entity)
    {
        throw std::runtime_error("Entity not found: " + entityId);
    }

    entity->setFieldValue(fieldName, value);
}

FieldValue *EntityManager::getFieldValue(const std::string &entityId, const std::string &fieldName)
{
    auto entity = getEntityById(entityId);
    if (!entity)
        return nullptr;
    return entity->getFieldValue(fieldName);
}

void EntityManager::validate(const std::string &entityId)
{
    auto entity = getEntityById(entityId);
    if (!entity)
    {
        throw std::runtime_error("Entity not found: " + entityId);
    }

    entity->validate();
}

std::vector<Entity *> EntityManager::query(const IEntityQuery &query) const
{
    return query.execute(*this);
}

const std::vector<Entity *> *EntityManager::getChildren(const std::string &parentId) const
{
    auto it = childrenIndex_.find(parentId);
    if (it != childrenIndex_.end())
    {
        return &(it->second);
    }
    return nullptr;
}

const std::vector<Entity *> &EntityManager::getParents() const
{
    return parents_;
}

void EntityManager::parseDataBundle(const std::unordered_map<std::string, std::string> &bundleContent)
{
    clear();

    for (const auto &pair : bundleContent)
    {
        const std::string &fileName = pair.first;
        const std::string &yamlContent = pair.second;

        YAML::Node root = YAML::Load(yamlContent);
        if (!root.IsMap())
        {
            throw std::runtime_error("Invalid data format in file: " + fileName);
        }

        for (auto it = root.begin(); it != root.end(); ++it)
        {
            std::string entityId = it->first.as<std::string>();
            YAML::Node entityNode = it->second;

            if (!entityNode["_schema"])
            {
                throw std::runtime_error("Entity '" + entityId + "' in file '" + fileName + "' is missing '_schema'");
            }

            std::string schemaName = entityNode["_schema"].as<std::string>();

            EntitySchema *schema = SchemaManager::instance().getEntitySchema(schemaName);
            if (!schema)
            {
                throw std::runtime_error("Entity '" + entityId + "' refers to unknown schema: " + schemaName);
            }

            auto entity = std::make_unique<Entity>(*schema);
            entity->setId(entityId);

            if (entityNode["_parentid"] && !entityNode["_parentid"].IsNull())
            {
                entity->setParentId(entityNode["_parentid"].as<std::string>());
            }

            for (auto fit = entityNode.begin(); fit != entityNode.end(); ++fit)
            {
                std::string key = fit->first.as<std::string>();
                if (key == "_schema" || key == "_parentid")
                    continue;

                const FieldSchema *fieldSchema = schema->getField(key);
                if (!fieldSchema)
                    throw std::runtime_error("Field '" + key + "' not defined in schema '" + schemaName + "'");

                FieldValue *fieldValue = entity->getFieldValue(key);
                if (!fieldValue)
                    throw std::runtime_error("Field '" + key + "' missing from entity");

                populateFieldValue(fieldValue, *fieldSchema, fit->second);
            }

            addEntity(std::move(entity));
        }
    }
}
