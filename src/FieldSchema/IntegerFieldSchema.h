#pragma once

#include "FieldSchema.h"
#include <optional>

struct IntegerFieldSchemaConfig : FieldSchemaConfig {
    std::optional<int64_t> minValue;
    std::optional<int64_t> maxValue;

    virtual ~IntegerFieldSchemaConfig() = default;
};


class IntegerFieldSchema : public FieldSchema {
public:
    explicit IntegerFieldSchema(const IntegerFieldSchemaConfig& config);

private:
    std::optional<int64_t> minValue_;
    std::optional<int64_t> maxValue_;
};

