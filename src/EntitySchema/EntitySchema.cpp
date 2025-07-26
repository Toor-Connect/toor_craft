#include "EntitySchema.h"
#include <stdexcept>

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

// ✅ now takes a raw pointer (SchemaManager owns it)
void EntitySchema::addChildSchema(const std::string &name, EntitySchema *child)
{
    if (!child)
        return;

    if (children_.find(name) != children_.end())
    {
        throw std::runtime_error(
            "Child schema with name '" + name + "' already exists in entity '" + name_ + "'");
    }
    children_[name] = child;
}

std::vector<std::string> EntitySchema::getChildrenNames() const
{
    std::vector<std::string> names;
    names.reserve(children_.size());
    for (const auto &pair : children_)
    {
        names.push_back(pair.first);
    }
    return names;
}

// ✅ return raw pointer instead of unique_ptr
EntitySchema *EntitySchema::getChildSchema(const std::string &name) const
{
    auto it = children_.find(name);
    if (it != children_.end())
    {
        return it->second;
    }
    return nullptr;
}

const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &EntitySchema::getFields() const
{
    return fields_;
}

void EntitySchema::addCommand(std::unique_ptr<Command> command)
{
    if (!command)
        return;
    const std::string &id = command->getId();
    commands_[id] = std::move(command);
}

Command *EntitySchema::getCommand(const std::string &commandId) const
{
    auto it = commands_.find(commandId);
    if (it != commands_.end())
    {
        return it->second.get();
    }
    return nullptr;
}
