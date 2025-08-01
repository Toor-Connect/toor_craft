#include "ObjectFieldSchema.h"

ObjectFieldSchema::ObjectFieldSchema(ObjectFieldSchemaConfig config)
    : FieldSchema(std::move(config))
{
}

void ObjectFieldSchema::addField(std::unique_ptr<FieldSchema> field)
{
    std::string fieldName = field->getName();
    fields_[fieldName] = std::move(field);
}

const FieldSchema *ObjectFieldSchema::getField(const std::string &name) const
{
    auto it = fields_.find(name);
    if (it != fields_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> ObjectFieldSchema::getFieldNames() const
{
    std::vector<std::string> names;
    names.reserve(fields_.size());
    for (const auto &pair : fields_)
    {
        names.push_back(pair.first);
    }
    return names;
}
