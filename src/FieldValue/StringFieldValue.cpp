#include "StringFieldValue.h"

StringFieldValue::StringFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void StringFieldValue::setValueFromString(const std::string &val)
{
    value_ = val;
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
