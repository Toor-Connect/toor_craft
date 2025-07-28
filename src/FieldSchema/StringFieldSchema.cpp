#include "StringFieldSchema.h"

StringFieldSchema::StringFieldSchema(StringFieldSchemaConfig config)
    : FieldSchema(std::move(config))
{
}
