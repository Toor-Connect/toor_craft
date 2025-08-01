#include "ReferenceFieldSchema.h"
#include <nlohmann/json.hpp>

ReferenceFieldSchema::ReferenceFieldSchema(ReferenceFieldSchemaConfig config)
    : FieldSchema(std::move(config)),
      targetEntityName_(config.targetEntityName)
{
}

std::string ReferenceFieldSchema::toJson() const
{
  nlohmann::json j;
  j["type"] = "reference";
  j["target"] = targetEntityName_;
  j["required"] = isRequired();
  if (getAlias())
    j["alias"] = *getAlias();
  return j.dump();
}
