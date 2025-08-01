#pragma once
#include "FieldSchema.h"

struct StringFieldSchemaConfig : FieldSchemaConfig
{
};

class StringFieldSchema : public FieldSchema
{
public:
    explicit StringFieldSchema(StringFieldSchemaConfig config);
    std::string getTypeName() const override { return "string"; }
    std::string toJson() const override;
};
