#include "EntitySchema.h"

EntitySchema::EntitySchema(std::string name)
    : name_(std::move(name)) {}

const std::string &EntitySchema::getName() const
{
    return name_;
}

void EntitySchema::addField(std::unique_ptr<FieldSchema> field)
{
    fields_[field->getName()] = std::move(field);
}

const FieldSchema *EntitySchema::getField(const std::string &fieldName) const
{
    auto it = fields_.find(fieldName);
    if (it != fields_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void EntitySchema::addChildSchema(std::unique_ptr<EntitySchema> child)
{
    children_.push_back(std::move(child));
}

const std::vector<std::unique_ptr<EntitySchema>> &EntitySchema::getChildren() const
{
    return children_;
}

const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &EntitySchema::getFields() const
{
    return fields_;
}
