#pragma once
#include "FieldSchema.h"
#include <vector>
#include <string>

struct EnumFieldSchemaConfig : FieldSchemaConfig
{
    std::vector<std::string> allowedValues;
};

class EnumFieldSchema : public FieldSchema
{
public:
    explicit EnumFieldSchema(EnumFieldSchemaConfig config);

    std::string getTypeName() const override { return "enum"; }
    const std::vector<std::string> &getAllowedValues() const { return allowedValues_; }

private:
    std::vector<std::string> allowedValues_;
};
