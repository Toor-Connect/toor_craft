#include "StringFieldValue.h"
#include <nlohmann/json.hpp>

StringFieldValue::StringFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void StringFieldValue::setValueFromString(const std::string &val)
{
    std::string processed = val;

    if (processed.size() >= 2 && processed.front() == '"' && processed.back() == '"')
    {
        processed = processed.substr(1, processed.size() - 2);
    }

    value_ = processed;
    validate();
}

std::string StringFieldValue::toString() const
{
    return value_.value_or("");
}

void StringFieldValue::validate() const
{
    return schema_.validate(value_);
}

bool StringFieldValue::isEmpty() const
{
    return !value_.has_value() || value_->empty();
}

std::string StringFieldValue::toJson() const
{
    nlohmann::json j;
    if (value_.has_value())
    {
        j = value_.value();
    }
    else
    {
        j = nullptr;
    }
    return j.dump();
}
