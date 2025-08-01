#pragma once
#include "FieldValue.h"
#include <optional>

class IntegerFieldValue : public FieldValue
{
public:
    explicit IntegerFieldValue(const FieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;

private:
    std::optional<int> value_;
};
