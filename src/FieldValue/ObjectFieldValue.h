#pragma once

#include "FieldValue.h"
#include <unordered_map>
#include <memory>
#include <string>

class ObjectFieldValue : public FieldValue
{
public:
    explicit ObjectFieldValue(const FieldSchema &schema);

    bool setValueFromString(const std::string &val, std::string &error) override;
    std::string toString() const override;
    bool validate(std::string &error) const override;

    void setFieldValue(const std::string &fieldName, std::unique_ptr<FieldValue> value);
    FieldValue *getFieldValue(const std::string &fieldName) const;
    bool hasFieldValue(const std::string &fieldName) const;

private:
    std::unordered_map<std::string, std::unique_ptr<FieldValue>> fieldValues_;
};