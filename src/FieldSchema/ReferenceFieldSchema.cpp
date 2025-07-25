#include "ReferenceFieldSchema.h"

ReferenceFieldSchema::ReferenceFieldSchema(const ReferenceFieldSchemaConfig& config)
    : FieldSchema(config), referencedEntityType_(config.referencedEntityType)
{
}

const std::string& ReferenceFieldSchema::getReferencedEntityType() const
{
    return referencedEntityType_;
}
