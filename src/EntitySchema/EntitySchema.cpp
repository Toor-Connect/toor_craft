#include "EntitySchema.h"
#include <nlohmann/json.hpp>

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
    children_[relationTag] = child;
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
    return it != children_.end() ? it->second : nullptr;
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

const std::unordered_map<std::string, std::unique_ptr<Command>> &EntitySchema::getCommands() const
{
    return commands_;
}

std::vector<std::string> EntitySchema::getCommandNames() const
{
    std::vector<std::string> names;
    names.reserve(commands_.size());
    for (const auto &pair : commands_)
    {
        names.push_back(pair.first);
    }
    return names;
}

std::string EntitySchema::toJson() const
{
    nlohmann::json j;
    j["name"] = name_;

    nlohmann::json fieldsJson = nlohmann::json::object();
    for (const auto &pair : fields_)
    {
        fieldsJson[pair.first] = nlohmann::json::parse(pair.second->toJson());
    }
    j["fields"] = fieldsJson;

    nlohmann::json childrenJson = nlohmann::json::object();
    for (const auto &pair : children_)
    {
        if (pair.second)
            childrenJson[pair.first] = pair.second->getName();
    }
    j["children"] = childrenJson;

    if (!commands_.empty())
    {
        nlohmann::json commandsJson = nlohmann::json::array();
        for (const auto &pair : commands_)
        {
            commandsJson.push_back(pair.first);
        }
        j["commands"] = commandsJson;
    }

    return j.dump();
}