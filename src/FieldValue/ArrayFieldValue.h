#pragma once
#include "FieldValue.h"
#include "ArrayFieldSchema.h"
#include <vector>
#include <memory>

class ArrayFieldValue : public FieldValue
{
public:
    explicit ArrayFieldValue(const ArrayFieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;
    void addElement(std::unique_ptr<FieldValue> value);
    const std::vector<std::unique_ptr<FieldValue>> &getElements() const { return elements_; }

private:
    const ArrayFieldSchema &getArraySchema() const
    {
        return static_cast<const ArrayFieldSchema &>(schema_);
    }

    std::vector<std::unique_ptr<FieldValue>> elements_;
};
