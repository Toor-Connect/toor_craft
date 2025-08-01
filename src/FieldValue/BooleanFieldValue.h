#pragma once
#include "FieldValue.h"
#include <optional>

class BooleanFieldValue : public FieldValue
{
public:
    explicit BooleanFieldValue(const FieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;

private:
    std::optional<bool> value_;
};
