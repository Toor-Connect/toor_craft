#include "IntegerFieldValue.h"
#include <string>

IntegerFieldValue::IntegerFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void IntegerFieldValue::setValueFromString(const std::string &val)
{
    try
    {
        value_ = std::stoi(val);
    }
    catch (...)
    {
        throw std::runtime_error("Invalid integer format: " + val);
    }

    validate();
}

std::string IntegerFieldValue::toString() const
{
    return value_ ? std::to_string(*value_) : "";
}

void IntegerFieldValue::validate() const
{
    if (!value_)
        return;
    return schema_.validate(std::optional<std::string>(std::to_string(*value_)));
}

bool IntegerFieldValue::isEmpty() const
{
    return !value_.has_value();
}
