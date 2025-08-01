#pragma once

#include "FieldValue.h"
#include <unordered_map>
#include <memory>
#include <string>

class ObjectFieldValue : public FieldValue
{
public:
    explicit ObjectFieldValue(const FieldSchema &schema);

    void setValueFromString(const std::string &val) override;
    std::string toString() const override;
    void validate() const override;
    bool isEmpty() const override;

    void setFieldValue(const std::string &fieldName, std::unique_ptr<FieldValue> value);
    FieldValue *getFieldValue(const std::string &fieldName) const;
    bool hasFieldValue(const std::string &fieldName) const;
    std::string toJson() const override;

private:
    std::unordered_map<std::string, std::unique_ptr<FieldValue>> fieldValues_;
};