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
    return it != fields_.end() ? it->second.get() : nullptr;
}

void EntitySchema::addChildSchema(const std::string &relationTag, EntitySchema *child)
{
    if (!child)
        return;
    if (children_.find(relationTag) != children_.end())
    {
        throw std::runtime_error("Child relation '" + relationTag + "' already exists in entity '" + name_ + "'");
    }
    children_[relationTag] = ChildRelation{relationTag, child};
}

std::vector<std::string> EntitySchema::getChildrenTags() const
{
    std::vector<std::string> tags;
    tags.reserve(children_.size());
    for (const auto &pair : children_)
    {
        tags.push_back(pair.first);
    }
    return tags;
}

EntitySchema *EntitySchema::getChildSchema(const std::string &relationTag) const
{
    auto it = children_.find(relationTag);
    return it != children_.end() ? it->second.schema : nullptr;
}

const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &EntitySchema::getFields() const
{
    return fields_;
}

void EntitySchema::addCommand(std::unique_ptr<Command> command)
{
    if (!command)
        return;
    commands_[command->getId()] = std::move(command);
}

Command *EntitySchema::getCommand(const std::string &commandId) const
{
    auto it = commands_.find(commandId);
    return it != commands_.end() ? it->second.get() : nullptr;
}
