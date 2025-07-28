#include "ArrayFieldValue.h"
#include "ArrayFieldSchema.h"
#include <sstream>

ArrayFieldValue::ArrayFieldValue(const ArrayFieldSchema &schema)
    : FieldValue(schema) {}

void ArrayFieldValue::addElement(std::unique_ptr<FieldValue> value)
{
    if (value)
    {
        elements_.push_back(std::move(value));
    }
}

bool ArrayFieldValue::setValueFromString(const std::string &val, std::string &error)
{
    // ❌ Arrays can’t be directly set from a string
    error = "Cannot set an array field from a raw string.";
    return false;
}

std::string ArrayFieldValue::toString() const
{
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements_.size(); ++i)
    {
        oss << elements_[i]->toString();
        if (i + 1 < elements_.size())
            oss << ", ";
    }
    oss << "]";
    return oss.str();
}

bool ArrayFieldValue::validate(std::string &error) const
{
    for (const auto &element : elements_)
    {
        if (!element->validate(error))
        {
            return false;
        }
    }
    return true;
}
