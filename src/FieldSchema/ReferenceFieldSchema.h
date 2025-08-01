#pragma once
#include "FieldSchema.h"

struct ReferenceFieldSchemaConfig : FieldSchemaConfig
{
    std::string targetEntityName;
};

class ReferenceFieldSchema : public FieldSchema
{
public:
    explicit ReferenceFieldSchema(ReferenceFieldSchemaConfig config);
    std::string getTypeName() const override { return "reference"; }
    const std::string &getTargetEntityName() const { return targetEntityName_; }
    std::string toJson() const override;

private:
    std::string targetEntityName_;
};
