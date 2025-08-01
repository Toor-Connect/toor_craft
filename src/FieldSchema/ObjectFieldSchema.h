#pragma once

#include "FieldSchema.h"
#include <unordered_map>
#include <memory>

struct ObjectFieldSchemaConfig : FieldSchemaConfig
{
};

class ObjectFieldSchema : public FieldSchema
{
public:
    explicit ObjectFieldSchema(ObjectFieldSchemaConfig config);

    std::string getTypeName() const override { return "object"; }

    void addField(std::unique_ptr<FieldSchema> field);

    const FieldSchema *getField(const std::string &name) const;
    const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &getFields() const { return fields_; }

    std::vector<std::string> getFieldNames() const;

private:
    std::unordered_map<std::string, std::unique_ptr<FieldSchema>> fields_;
};
