#include "IntegerFieldValue.h"
#include <string>

IntegerFieldValue::IntegerFieldValue(const FieldSchema& schema)
    : FieldValue(schema) {}

bool IntegerFieldValue::setValueFromString(const std::string& val, std::string& error) {
    try {
        value_ = std::stoi(val);
    } catch (...) {
        error = "Invalid integer format";
        return false;
    }
    return validate(error);
}

std::string IntegerFieldValue::toString() const {
    return value_ ? std::to_string(*value_) : "";
}

bool IntegerFieldValue::validate(std::string& error) const {
    if (!value_) return true; // or false if required
    return schema_.validate(std::optional<std::string>(std::to_string(*value_)), error);
}
