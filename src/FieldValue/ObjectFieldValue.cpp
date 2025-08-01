#include "ObjectFieldValue.h"
#include "ObjectFieldSchema.h"
#include "FieldValueFactory.h"
#include "FieldSchema.h"
#include <stdexcept>
#include <sstream>
#include <nlohmann/json.hpp>

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
    try
    {
        nlohmann::json parsed = nlohmann::json::parse(val);

        if (!parsed.is_object())
        {
            throw std::runtime_error("ObjectFieldValue expected a JSON object but got: " + val);
        }

        const auto &objSchema = static_cast<const ObjectFieldSchema &>(getSchema());

        for (auto it = parsed.begin(); it != parsed.end(); ++it)
        {
            const std::string &fieldName = it.key();
            const nlohmann::json &fieldJson = it.value();

            const FieldSchema *fieldSchema = objSchema.getField(fieldName);
            if (!fieldSchema)
            {
                throw std::runtime_error("Unknown field '" + fieldName + "' in object JSON.");
            }

            std::unique_ptr<FieldValue> fieldValue =
                FieldValueFactory::instance().create(fieldSchema->getTypeName(), *fieldSchema);

            fieldValue->setValueFromString(fieldJson.dump());

            setFieldValue(fieldName, std::move(fieldValue));
        }

        validate();
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("Failed to set ObjectFieldValue from string: ") + e.what());
    }
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

        oss << "\"" << pair.first << "\": ";

        const std::string valStr = pair.second->toString();

        if (!valStr.empty() && (valStr.front() == '{' || valStr.front() == '['))
        {
            oss << valStr;
        }
        else
        {
            oss << "\"" << valStr << "\"";
        }
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
    for (const auto &[name, field] : fieldValues_)
    {
        if (!field->isEmpty())
            return false;
    }
    return true;
}

std::string ObjectFieldValue::toJson() const
{
    nlohmann::json j = nlohmann::json::object();
    for (const auto &[fieldName, fieldValue] : fieldValues_)
    {
        j[fieldName] = nlohmann::json::parse(fieldValue->toJson());
    }
    return j.dump();
}
