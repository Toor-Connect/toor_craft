#pragma once

#include "FieldSchema.h"
#include <optional>


struct BooleanFieldSchemaConfig : FieldSchemaConfig {
    virtual ~BooleanFieldSchemaConfig() = default;
};

class BooleanFieldSchema : public FieldSchema {
public:
    explicit BooleanFieldSchema(const BooleanFieldSchemaConfig& config);
    std::string getTypeName() const override { return "boolean"; }
};
