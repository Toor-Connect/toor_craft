#include "EnumFieldValue.h"
#include <nlohmann/json.hpp>

EnumFieldValue::EnumFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void EnumFieldValue::setValueFromString(const std::string &val)
{
    std::string processed = val;

    if (processed.size() >= 2 && processed.front() == '"' && processed.back() == '"')
    {
        processed = processed.substr(1, processed.size() - 2);
    }

    value_ = processed;
    validate();
}

std::string EnumFieldValue::toString() const
{
    return value_.value_or("");
}

void EnumFieldValue::validate() const
{
    return schema_.validate(value_);
}

bool EnumFieldValue::isEmpty() const
{
    return !value_.has_value();
}

std::string EnumFieldValue::toJson() const
{
    nlohmann::json j;
    if (value_.has_value())
        j = *value_;
    else
        j = nullptr;
    return j.dump();
}