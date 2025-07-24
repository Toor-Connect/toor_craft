#pragma once

#include "Field.h"
#include <optional>

struct FloatFieldConfig : FieldConfig {
    std::optional<double> minValue;
    std::optional<double> maxValue;

    virtual ~FloatFieldConfig() = default;
};

class FloatField : public Field {
public:
    explicit FloatField(const FloatFieldConfig& config);

private:
    std::optional<double> minValue_;
    std::optional<double> maxValue_;
};