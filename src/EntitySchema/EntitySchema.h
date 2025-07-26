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
    struct ChildRelation
    {
        std::string relationTag; // e.g. "devices"
        EntitySchema *schema;    // raw pointer, SchemaManager owns lifetime
    };

    explicit EntitySchema(std::string name);

    const std::string &getName() const;

    // Add a field schema to this entity
    void addField(std::unique_ptr<FieldSchema> field);

    // Get field schema by name, nullptr if not found
    const FieldSchema *getField(const std::string &fieldName) const;

    // Add a child relation (SchemaManager owns the memory)
    void addChildSchema(const std::string &relationTag, EntitySchema *child);

    // Get all child relation tags
    std::vector<std::string> getChildrenTags() const;

    // Get child schema by relation tag (nullptr if not found)
    EntitySchema *getChildSchema(const std::string &relationTag) const;

    const std::unordered_map<std::string, std::unique_ptr<FieldSchema>> &getFields() const;

    // Add a command to this schema
    void addCommand(std::unique_ptr<Command> command);

    // Get a command by id (nullptr if not found)
    Command *getCommand(const std::string &commandId) const;

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<FieldSchema>> fields_;

    // ✅ now store a ChildRelation, keyed by relation tag
    std::unordered_map<std::string, ChildRelation> children_;

    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};
