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

    // Add a child (just store a pointer, SchemaManager owns the memory)
    void addChildSchema(const std::string &name, EntitySchema *child);

    // Get all children names
    std::vector<std::string> getChildrenNames() const;

    // Get child schema by name (nullptr if not found)
    EntitySchema *getChildSchema(const std::string &name) const;

    const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &getFields() const;

    // Add a command to this schema
    void addCommand(std::unique_ptr<Command> command);

    // Get a command by id (nullptr if not found)
    Command *getCommand(const std::string &commandId) const;

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<FieldSchema>> fields_;
    std::unordered_map<std::string, EntitySchema *> children_; // 🔄 now raw pointers only
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};
