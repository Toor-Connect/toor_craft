#pragma once
#include "FieldValue.h"
#include <optional>

class BooleanFieldValue : public FieldValue {
public:
    explicit BooleanFieldValue(const FieldSchema& schema);

    bool setValueFromString(const std::string& val, std::string& error) override;
    std::string toString() const override;
    bool validate(std::string& error) const override;

private:
    std::optional<bool> value_;
};
