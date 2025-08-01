#pragma once
#include "FieldValue.h"
#include <optional>

class FloatFieldValue : public FieldValue
{
public:
    explicit FloatFieldValue(const FieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;
    std::string toJson() const override;

private:
    std::optional<float> value_;
};
