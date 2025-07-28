#pragma once
#include "FieldSchema.h"

struct BooleanFieldSchemaConfig : FieldSchemaConfig
{
};

class BooleanFieldSchema : public FieldSchema
{
public:
    explicit BooleanFieldSchema(BooleanFieldSchemaConfig config);

    std::string getTypeName() const override { return "boolean"; }
};
