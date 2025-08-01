#include "EnumFieldValue.h"

EnumFieldValue::EnumFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void EnumFieldValue::setValueFromString(const std::string &val)
{
    value_ = val;
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
