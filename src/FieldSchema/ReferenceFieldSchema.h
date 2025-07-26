#pragma once

#include "FieldSchema.h"
#include <optional>
#include <string>

struct ReferenceFieldSchemaConfig : FieldSchemaConfig
{
    std::string targetEntityName; // type of entity this reference points to

    ReferenceFieldSchemaConfig() = default;
    // Add constructors or default values as needed
};

class ReferenceFieldSchema : public FieldSchema
{
public:
    explicit ReferenceFieldSchema(const ReferenceFieldSchemaConfig &config);

    const std::string &getTargetEntityName() const;

    std::string getTypeName() const override { return "reference"; }

private:
    std::string targetEntityName_;
};