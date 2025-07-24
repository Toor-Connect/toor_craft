#include "FloatFieldValue.h"
#include <string>

FloatFieldValue::FloatFieldValue(const FieldSchema& schema)
    : FieldValue(schema) {}

bool FloatFieldValue::setValueFromString(const std::string& val, std::string& error) {
    try {
        value_ = std::stof(val);
    } catch (...) {
        error = "Invalid float format";
        return false;
    }
    return validate(error);
}

std::string FloatFieldValue::toString() const {
    return value_ ? std::to_string(*value_) : "";
}

bool FloatFieldValue::validate(std::string& error) const {
    if (!value_) return true; // or false if required
    return schema_.validate(std::optional<std::string>(std::to_string(*value_)), error);
}
