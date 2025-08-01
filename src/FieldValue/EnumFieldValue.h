#pragma once
#include "FieldValue.h"
#include <optional>

class EnumFieldValue : public FieldValue
{
public:
    explicit EnumFieldValue(const FieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;
    std::string toJson() const override;

private:
    std::optional<std::string> value_;
};
