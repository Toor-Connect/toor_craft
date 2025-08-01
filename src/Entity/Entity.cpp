#include "Entity.h"
#include "FieldValueFactory.h" // Use FieldValueFactory, not FieldSchemaFactory
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Entity::Entity(const EntitySchema &schema)
    : schema_(schema)
{
    // Iterate over the fields of the schema
    for (const auto &[fieldName, fieldSchemaPtr] : schema_.getFields())
    {
        // Create FieldValue using FieldValueFactory from the field schema
        auto fieldValue = FieldValueFactory::instance().create(fieldSchemaPtr->getTypeName(), *fieldSchemaPtr);
        fieldValues_.emplace(fieldName, std::move(fieldValue));
    }
}

const EntitySchema &Entity::getSchema() const
{
    return schema_;
}

FieldValue *Entity::getFieldValue(const std::string &fieldName)
{
    auto it = fieldValues_.find(fieldName);
    if (it != fieldValues_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void Entity::setFieldValue(const std::string &fieldName, const std::string &value)
{
    auto *fieldValue = getFieldValue(fieldName);
    if (!fieldValue)
    {
        throw std::runtime_error("Field not found: " + fieldName);
    }

    fieldValue->setValueFromString(value);
}

void Entity::validate() const
{
    for (const auto &[name, fieldValue] : fieldValues_)
    {
        if (fieldValue->getSchema().isRequired() && fieldValue->isEmpty())
        {
            throw std::runtime_error("Missing required field '" + name + "' in entity '" + _id + "'");
        }

        fieldValue->validate();
    }
}

void Entity::setId(const std::string &id)
{
    _id = id;
}

const std::string &Entity::getId() const
{
    return _id;
}

void Entity::setParentId(const std::string &parentId)
{
    _parentId = parentId;
}

const std::string &Entity::getParentId() const
{
    return _parentId;
}

std::unordered_map<std::string, std::string> Entity::getDict() const
{
    std::unordered_map<std::string, std::string> dict;
    for (const auto &[key, valuePtr] : fieldValues_)
    {
        dict[key] = valuePtr->toString();
    }
    return dict;
}

std::string Entity::getJson() const
{
    json entityJson;

    // Always include metadata
    entityJson["id"] = _id;
    entityJson["schema"] = schema_.getName();
    entityJson["parentId"] = _parentId.empty() ? json(nullptr) : json(_parentId);

    // Iterate through fields and convert each to proper JSON
    for (const auto &pair : fieldValues_)
    {
        const std::string &fieldName = pair.first;
        const auto &fieldValue = pair.second;

        if (fieldValue)
        {
            entityJson[fieldName] = json::parse(fieldValue->toJson());
        }
    }

    return entityJson.dump(2); // pretty print for readability
}