#include "BooleanFieldValue.h"
#include <algorithm>
#include <string>

BooleanFieldValue::BooleanFieldValue(const FieldSchema& schema)
    : FieldValue(schema) {}

bool BooleanFieldValue::setValueFromString(const std::string& val, std::string& error) {
    std::string lowerVal = val;
    std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), ::tolower);

    if (lowerVal == "true" || lowerVal == "1") {
        value_ = true;
    } else if (lowerVal == "false" || lowerVal == "0") {
        value_ = false;
    } else {
        error = "Invalid boolean value";
        return false;
    }
    return validate(error);
}

std::string BooleanFieldValue::toString() const {
    return value_ ? "true" : "false";
}

bool BooleanFieldValue::validate(std::string& error) const {
    if (!value_) return true; // or false if required
    return schema_.validate(std::optional<std::string>(value_ ? "true" : "false"), error);
}
