#pragma once
#include "FieldValue.h"
#include <optional>

class StringFieldValue : public FieldValue {
public:
    explicit StringFieldValue(const FieldSchema& schema);

    bool setValueFromString(const std::string& val, std::string& error) override;
    std::string toString() const override;
    bool validate(std::string& error) const override;

private:
    std::optional<std::string> value_;
};
