#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "EntitySchema.h"
#include "FieldValue.h"

class EntitySchema; // Forward declaration

enum class EntityState
{
    Unchanged,
    Added,
    Modified,
    Deleted
};

class Entity
{
public:
    explicit Entity(const EntitySchema &schema);
    const EntitySchema &getSchema() const;
    FieldValue *getFieldValue(const std::string &fieldName);
    void setFieldValue(const std::string &fieldName, const std::string &value);
    void validate() const;
    void setId(const std::string &id);
    const std::string &getId() const;
    void setParentId(const std::string &parentId);
    const std::string &getParentId() const;
    std::unordered_map<std::string, std::string> getDict() const;
    std::string getJson() const;
    void setState(EntityState newState) { state_ = newState; }
    EntityState getState() const { return state_; }
    bool isDeleted() const { return state_ == EntityState::Deleted; }

private:
    const EntitySchema &schema_;
    std::unordered_map<std::string, std::unique_ptr<FieldValue>> fieldValues_;
    std::string _id;
    std::string _parentId;
    EntityState state_ = EntityState::Unchanged;
};
