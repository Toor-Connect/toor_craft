#pragma once

#include "FieldSchema.h"
#include <string>
#include <vector>

struct EnumFieldSchemaConfig : FieldSchemaConfig {
    std::vector<std::string> allowedValues;

    virtual ~EnumFieldSchemaConfig() = default;
};

class EnumFieldSchema : public FieldSchema {
public:
    explicit EnumFieldSchema(const EnumFieldSchemaConfig& config);
    std::string getTypeName() const override { return "enum"; }

private:
    std::vector<std::string> allowedValues_;
};