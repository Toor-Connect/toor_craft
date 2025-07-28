#include "ReferenceFieldSchema.h"

ReferenceFieldSchema::ReferenceFieldSchema(ReferenceFieldSchemaConfig config)
    : FieldSchema(std::move(config)),
      targetEntityName_(config.targetEntityName)
{
}
