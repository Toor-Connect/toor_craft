#include "BooleanFieldValue.h"
#include <algorithm>
#include <string>
#include <nlohmann/json.hpp>

BooleanFieldValue::BooleanFieldValue(const FieldSchema &schema)
    : FieldValue(schema) {}

void BooleanFieldValue::setValueFromString(const std::string &val)
{
    std::string lowerVal = val;
    std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), ::tolower);

    if (lowerVal == "true" || lowerVal == "1")
    {
        value_ = true;
    }
    else if (lowerVal == "false" || lowerVal == "0")
    {
        value_ = false;
    }
    else
    {
        throw std::runtime_error("Invalid boolean value: " + val);
    }

    validate(); // validate will also throw if something is wrong
}

std::string BooleanFieldValue::toString() const
{
    return value_ ? "true" : "false";
}
void BooleanFieldValue::validate() const
{
    if (!value_)
        return;

    schema_.validate(std::optional<std::string>(value_ ? "true" : "false"));
}

bool BooleanFieldValue::isEmpty() const
{
    return !value_.has_value();
}

std::string BooleanFieldValue::toJson() const
{
    nlohmann::json j;
    if (value_.has_value())
        j = *value_;
    else
        j = nullptr;
    return j.dump();
}
