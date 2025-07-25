#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "EntitySchema.h"
#include "FieldValue.h"

class EntitySchema; // Forward declaration

class Entity {
public:
    explicit Entity(const EntitySchema& schema);

    const EntitySchema& getSchema() const;

    // Get FieldValue by field name; nullptr if not found
    FieldValue* getFieldValue(const std::string& fieldName);

    // Set field value by name from string, returns false and error message on failure
    bool setFieldValue(const std::string& fieldName, const std::string& value, std::string& error);

    // Validate all fields, returns false and first error encountered
    bool validate(std::string& error) const;

    // ID and parent ID management
    void setId(const std::string& id);
    const std::string& getId() const;

    void setParentId(const std::string& parentId);
    const std::string& getParentId() const;

private:
    const EntitySchema& schema_;
    std::unordered_map<std::string, std::unique_ptr<FieldValue>> fieldValues_;

    std::string _id;
    std::string _parentId;
};
