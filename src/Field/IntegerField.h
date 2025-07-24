#pragma once

#include "Field.h"
#include <optional>

struct IntegerFieldConfig : FieldConfig {
    std::optional<int64_t> minValue;
    std::optional<int64_t> maxValue;

    virtual ~IntegerFieldConfig() = default;
};


class IntegerField : public Field {
public:
    explicit IntegerField(const IntegerFieldConfig& config);

private:
    std::optional<int64_t> minValue_;
    std::optional<int64_t> maxValue_;
};

