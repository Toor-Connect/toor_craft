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

    // Add a field schema to this entity
    void addField(std::unique_ptr<FieldSchema> field);

    // Get field schema by name, nullptr if not found
    const FieldSchema *getField(const std::string &fieldName) const;

    // Add a child entity schema
    void addChildSchema(std::unique_ptr<EntitySchema> child);

    // Get all child schemas
    const std::vector<std::unique_ptr<EntitySchema>> &getChildren() const;

    const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &getFields() const;

    // Add a command to this schema
    void addCommand(std::unique_ptr<Command> command);

    // Get a command by id (nullptr if not found)
    Command *getCommand(const std::string &commandId) const;

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<FieldSchema>> fields_;
    std::vector<std::unique_ptr<EntitySchema>> children_;
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};
