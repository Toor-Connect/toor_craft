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

void ArrayFieldValue::setValueFromString(const std::string &val)
{
    throw std::runtime_error("Cannot set an array field from a raw string.");
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

void ArrayFieldValue::validate() const
{
    for (const auto &element : elements_)
    {
        element->validate();
    }
}

bool ArrayFieldValue::isEmpty() const
{
    return elements_.empty();
}
