#include "StringFieldSchema.h"
#include <nlohmann/json.hpp>

StringFieldSchema::StringFieldSchema(StringFieldSchemaConfig config)
    : FieldSchema(std::move(config))
{
}

std::string StringFieldSchema::toJson() const
{
    nlohmann::json j;
    j["type"] = "string";
    j["required"] = isRequired();
    if (getAlias())
        j["alias"] = *getAlias();
    return j.dump();
}
