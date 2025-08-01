#pragma once
#include <string>
#include <optional>
#include "FieldSchema.h"

class FieldValue
{
public:
    explicit FieldValue(const FieldSchema &schema) : schema_(schema) {}
    virtual ~FieldValue() = default;
    FieldValue(const FieldValue &) = delete;
    FieldValue &operator=(const FieldValue &) = delete;

    const FieldSchema &getSchema() const { return schema_; }

    virtual void setValueFromString(const std::string &val) = 0;
    virtual std::string toString() const = 0;
    virtual void validate() const = 0;
    virtual bool isEmpty() const = 0;

protected:
    const FieldSchema &schema_;
};
