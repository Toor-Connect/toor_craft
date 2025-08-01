#include "ArrayFieldValue.h"
#include "FieldValueFactory.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

ArrayFieldValue::ArrayFieldValue(const ArrayFieldSchema &schema)
    : FieldValue(schema) {}

void ArrayFieldValue::setValueFromString(const std::string &val)
{
    elements_.clear();

    try
    {
        json parsed = json::parse(val);

        if (!parsed.is_array())
        {
            throw std::runtime_error("ArrayFieldValue expected a JSON array but got: " + val);
        }

        const FieldSchema &elementSchema = getArraySchema().getElementSchema();

        for (const auto &item : parsed)
        {
            std::string itemStr = item.dump();

            std::unique_ptr<FieldValue> elementValue =
                FieldValueFactory::instance().create(elementSchema.getTypeName(), elementSchema);

            elementValue->setValueFromString(itemStr);

            addElement(std::move(elementValue));
        }

        validate();
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("Failed to set ArrayFieldValue from string: ") + e.what());
    }
}

std::string ArrayFieldValue::toString() const
{
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements_.size(); ++i)
    {
        oss << elements_[i]->toString();
        if (i + 1 < elements_.size())
        {
            oss << ", ";
        }
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

void ArrayFieldValue::addElement(std::unique_ptr<FieldValue> value)
{
    if (value)
    {
        elements_.push_back(std::move(value));
    }
}

std::string ArrayFieldValue::toJson() const
{
    nlohmann::json j = nlohmann::json::array();
    for (const auto &element : elements_)
    {
        j.push_back(nlohmann::json::parse(element->toJson()));
    }
    return j.dump();
}