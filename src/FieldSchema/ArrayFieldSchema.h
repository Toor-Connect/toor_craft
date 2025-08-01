#pragma once
#include "FieldSchema.h"
#include <memory>

struct ArrayFieldSchemaConfig : FieldSchemaConfig
{
    std::unique_ptr<FieldSchema> elementSchema;
};

class ArrayFieldSchema : public FieldSchema
{
public:
    explicit ArrayFieldSchema(ArrayFieldSchemaConfig &&config);
    std::string getTypeName() const override { return "array"; }
    const FieldSchema &getElementSchema() const { return *elementSchema_; }
    std::string toJson() const override;

private:
    std::unique_ptr<FieldSchema> elementSchema_;
};
