#include "ReferenceFieldSchema.h"

ReferenceFieldSchema::ReferenceFieldSchema(const ReferenceFieldSchemaConfig &config)
    : FieldSchema(config), targetEntityName_(config.targetEntityName)
{
}

const std::string &ReferenceFieldSchema::getTargetEntityName() const
{
    return targetEntityName_;
}
