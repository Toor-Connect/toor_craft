#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "FieldSchema.h"
#include "Command.h"

class EntitySchema
{
public:
    explicit EntitySchema(std::string name);
    const std::string &getName() const;
    void addField(std::unique_ptr<FieldSchema> field);
    const FieldSchema *getField(const std::string &fieldName) const;
    void addChildSchema(const std::string &relationTag, EntitySchema *child);
    std::vector<std::string> getChildrenTags() const;
    EntitySchema *getChildSchema(const std::string &relationTag) const;
    const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &getFields() const;
    void addCommand(std::unique_ptr<Command> command);
    Command *getCommand(const std::string &commandId) const;
    const std::unordered_map<std::string, std::unique_ptr<Command>> &getCommands() const;
    std::vector<std::string> getCommandNames() const;

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<FieldSchema>> fields_;
    std::unordered_map<std::string, EntitySchema *> children_;
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};
