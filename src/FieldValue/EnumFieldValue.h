#pragma once
#include "FieldValue.h"
#include <optional>

class EnumFieldValue : public FieldValue {
public:
    explicit EnumFieldValue(const FieldSchema& schema);

    bool setValueFromString(const std::string& val, std::string& error) override;
    std::string toString() const override;
    bool validate(std::string& error) const override;

private:
    std::optional<std::string> value_;
};
