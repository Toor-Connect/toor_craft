#pragma once
#include "FieldValue.h"
#include <optional>

class StringFieldValue : public FieldValue
{
public:
    explicit StringFieldValue(const FieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;

private:
    std::optional<std::string> value_;
};
