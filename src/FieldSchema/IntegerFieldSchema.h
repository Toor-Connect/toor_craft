#pragma once

#include "FieldSchema.h"
#include <optional>

struct IntegerFieldSchemaConfig : FieldSchemaConfig
{
    std::optional<int64_t> minValue;
    std::optional<int64_t> maxValue;

    virtual ~IntegerFieldSchemaConfig() = default;
};

class IntegerFieldSchema : public FieldSchema
{
public:
    explicit IntegerFieldSchema(const IntegerFieldSchemaConfig &config);
    std::string getTypeName() const override { return "integer"; }
    const std::optional<int64_t> &getMinValue() const { return minValue_; }
    const std::optional<int64_t> &getMaxValue() const
    {
        return maxValue_;
    }

private:
    std::optional<int64_t> minValue_;
    std::optional<int64_t> maxValue_;
};
