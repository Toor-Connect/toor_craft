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

bool ObjectFieldValue::setValueFromString(const std::string &val, std::string &error)
{
    // ❌ Objects cannot be directly set from a single string
    error = "Cannot assign a raw string value to an object field.";
    return false;
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

bool ObjectFieldValue::validate(std::string &error) const
{
    const auto &objSchema = static_cast<const ObjectFieldSchema &>(getSchema());

    for (const auto &pair : objSchema.getFields())
    {
        const std::string &fieldName = pair.first;
        const FieldSchema *fieldSchema = pair.second.get();

        // Check required fields exist
        if (fieldSchema->isRequired() && !hasFieldValue(fieldName))
        {
            error = "Missing required field '" + fieldName + "' in object.";
            return false;
        }

        // Validate nested fields recursively
        if (hasFieldValue(fieldName))
        {
            FieldValue *val = getFieldValue(fieldName);
            if (!val->validate(error))
            {
                return false;
            }
        }
    }
    return true;
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
