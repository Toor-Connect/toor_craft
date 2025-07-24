#include "StringFieldValue.h"

StringFieldValue::StringFieldValue(const FieldSchema& schema)
    : FieldValue(schema) {}

bool StringFieldValue::setValueFromString(const std::string& val, std::string& error) {
    value_ = val;
    return validate(error);
}

std::string StringFieldValue::toString() const {
    return value_.value_or("");
}

bool StringFieldValue::validate(std::string& error) const {
    return schema_.validate(value_, error);
}
