#pragma once

#include "FieldSchema.h"
#include <optional>
#include <string>

struct StringFieldSchemaConfig : FieldSchemaConfig {
    virtual ~StringFieldSchemaConfig() = default;
};

class StringFieldSchema : public FieldSchema {
public:
    explicit StringFieldSchema(const StringFieldSchemaConfig& config);
    std::string getTypeName() const override { return "string"; }
};
