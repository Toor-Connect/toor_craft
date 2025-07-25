#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Entity.h"

class IEntityQuery {
public:
    virtual ~IEntityQuery() = default;
    virtual std::vector<Entity*> execute(const EntityManager& manager) const = 0;
};


class EntityManager {
public:
    // Singleton access
    static EntityManager& instance();

    // Add an entity to the manager
    void addEntity(std::unique_ptr<Entity> entity);

    // Get entity by ID (nullptr if not found)
    Entity* getEntityById(const std::string& id) const;

    // Get children by parent ID (nullptr if no children)
    const std::vector<Entity*>* getChildren(const std::string& parentId) const;

    // Remove an entity by ID; returns true if removed
    bool removeEntity(const std::string& id);

    // Clear all entities
    void clear();

    // Set a field value of an entity by ID and field name
    bool setFieldValue(const std::string& entityId, const std::string& fieldName, const std::string& value, std::string& error);

    // Get a field value pointer of an entity by ID and field name (nullptr if not found)
    FieldValue* getFieldValue(const std::string& entityId, const std::string& fieldName);

    // Validate an entity by ID
    bool validateEntity(const std::string& entityId, std::string& error);

private:
    EntityManager() = default;
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Entity>> entities_;
    std::unordered_map<std::string, std::vector<Entity*>> childrenIndex_;
};
