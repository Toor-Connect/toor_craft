#include "ArrayFieldSchema.h"
#include <nlohmann/json.hpp>

ArrayFieldSchema::ArrayFieldSchema(ArrayFieldSchemaConfig &&config)
    : FieldSchema(config),
      elementSchema_(std::move(config.elementSchema))
{
    if (!elementSchema_)
    {
        throw std::runtime_error("ArrayFieldSchema requires an element schema.");
    }
}

std::string ArrayFieldSchema::toJson() const
{
    nlohmann::json j;
    j["type"] = "array";
    j["required"] = isRequired();
    if (getAlias())
        j["alias"] = *getAlias();

    if (elementSchema_)
    {
        j["element"] = nlohmann::json::parse(elementSchema_->toJson());
    }
    return j.dump();
}
