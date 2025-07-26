#include "EnumFieldValue.h"

EnumFieldValue::EnumFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

bool EnumFieldValue::setValueFromString(const std::string &val, std::string &error)
{
    value_ = val;
    return validate(error);
}

std::string EnumFieldValue::toString() const
{
    return value_.value_or("");
}

bool EnumFieldValue::validate(std::string &error) const
{
    return schema_.validate(value_, error);
}
