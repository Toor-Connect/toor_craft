#pragma once
#include "FieldValue.h"
#include "ArrayFieldSchema.h"
#include <vector>
#include <memory>

class ArrayFieldValue : public FieldValue
{
public:
    explicit ArrayFieldValue(const ArrayFieldSchema &schema);

    bool setValueFromString(const std::string &val, std::string &error) override;
    std::string toString() const override;
    bool validate(std::string &error) const override;

    // Add a new element to the array
    void addElement(std::unique_ptr<FieldValue> value);

    // Get elements
    const std::vector<std::unique_ptr<FieldValue>> &getElements() const { return elements_; }

private:
    const ArrayFieldSchema &getArraySchema() const
    {
        return static_cast<const ArrayFieldSchema &>(schema_);
    }

    std::vector<std::unique_ptr<FieldValue>> elements_;
};
