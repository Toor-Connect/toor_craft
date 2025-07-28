#include "ArrayFieldSchema.h"

ArrayFieldSchema::ArrayFieldSchema(ArrayFieldSchemaConfig &&config)
    : FieldSchema(config),
      elementSchema_(std::move(config.elementSchema))
{
    if (!elementSchema_)
    {
        throw std::runtime_error("ArrayFieldSchema requires an element schema.");
    }
}
