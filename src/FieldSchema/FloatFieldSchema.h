#pragma once

#include "FieldSchema.h"
#include <optional>

struct FloatFieldSchemaConfig : FieldSchemaConfig
{
    std::optional<double> minValue;
    std::optional<double> maxValue;

    virtual ~FloatFieldSchemaConfig() = default;
};

class FloatFieldSchema : public FieldSchema
{
public:
    explicit FloatFieldSchema(const FloatFieldSchemaConfig &config);
    std::string getTypeName() const override { return "float"; }
    const std::optional<double> &getMinValue() const { return minValue_; }
    const std::optional<double> &getMaxValue() const { return maxValue_; }

private:
    std::optional<double> minValue_;
    std::optional<double> maxValue_;
};