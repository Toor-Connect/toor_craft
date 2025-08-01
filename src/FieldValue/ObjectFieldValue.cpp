#include "ObjectFieldValue.h"
#include "ObjectFieldSchema.h"
#include "FieldValueFactory.h"
#include "FieldSchema.h"
#include <stdexcept>
#include <sstream>

ObjectFieldValue::ObjectFieldValue(const FieldSchema &schema)
    : FieldValue(schema)
{
    const auto &objSchema = static_cast<const ObjectFieldSchema &>(schema);
    for (const auto &[fieldName, fieldSchemaPtr] : objSchema.getFields())
    {
        auto value = FieldValueFactory::instance().create(fieldSchemaPtr->getTypeName(), *fieldSchemaPtr);
        fieldValues_.emplace(fieldName, std::move(value));
    }
}

void ObjectFieldValue::setValueFromString(const std::string &val)
{
    throw std::runtime_error("Cannot assign a raw string value to an object field.");
}

std::string ObjectFieldValue::toString() const
{
    std::ostringstream oss;
    oss << "{";

    bool first = true;
    for (const auto &pair : fieldValues_)
    {
        if (!first)
            oss << ", ";
        first = false;

        oss << "\"" << pair.first << "\": \"" << pair.second->toString() << "\"";
    }

    oss << "}";
    return oss.str();
}

void ObjectFieldValue::validate() const
{
    const auto &objSchema = static_cast<const ObjectFieldSchema &>(getSchema());

    for (const auto &pair : objSchema.getFields())
    {
        const std::string &fieldName = pair.first;
        const FieldSchema *fieldSchema = pair.second.get();

        if (fieldSchema->isRequired() && !hasFieldValue(fieldName))
        {
            throw std::runtime_error("Missing required field '" + fieldName + "' in object.");
        }

        if (hasFieldValue(fieldName))
        {
            FieldValue *val = getFieldValue(fieldName);
            val->validate();
        }
    }
}

void ObjectFieldValue::setFieldValue(const std::string &fieldName, std::unique_ptr<FieldValue> value)
{
    fieldValues_[fieldName] = std::move(value);
}

FieldValue *ObjectFieldValue::getFieldValue(const std::string &fieldName) const
{
    auto it = fieldValues_.find(fieldName);
    return (it != fieldValues_.end()) ? it->second.get() : nullptr;
}

bool ObjectFieldValue::hasFieldValue(const std::string &fieldName) const
{
    return fieldValues_.find(fieldName) != fieldValues_.end();
}

bool ObjectFieldValue::isEmpty() const
{
    // Consider object empty if ALL its fields are empty
    for (const auto &[name, field] : fieldValues_)
    {
        if (!field->isEmpty())
            return false;
    }
    return true;
}
