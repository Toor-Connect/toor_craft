#include "FloatFieldValue.h"
#include <string>

FloatFieldValue::FloatFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void FloatFieldValue::setValueFromString(const std::string &val)
{
    try
    {
        value_ = std::stof(val);
    }
    catch (...)
    {
        throw std::runtime_error("Invalid float format: " + val);
    }

    validate();
}

std::string FloatFieldValue::toString() const
{
    return value_ ? std::to_string(*value_) : "";
}

void FloatFieldValue::validate() const
{
    if (!value_)
        return;
    return schema_.validate(std::optional<std::string>(std::to_string(*value_)));
}

bool FloatFieldValue::isEmpty() const
{
    return !value_.has_value();
}
