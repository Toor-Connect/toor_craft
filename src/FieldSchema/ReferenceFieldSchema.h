#pragma once

#include "FieldSchema.h"
#include <optional>
#include <string>

struct ReferenceFieldSchemaConfig : FieldSchemaConfig {
    std::string referencedEntityType; // type of entity this reference points to

    ReferenceFieldSchemaConfig() = default;
    // Add constructors or default values as needed
};

class ReferenceFieldSchema : public FieldSchema {
public:
    explicit ReferenceFieldSchema(const ReferenceFieldSchemaConfig& config);

    const std::string& getReferencedEntityType() const;

    std::string getTypeName() const override { return "reference"; }

private:
    std::string referencedEntityType_;
};