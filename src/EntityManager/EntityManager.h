#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Entity.h"

class EntityManager {
public:
    // Get the singleton instance
    static EntityManager& instance();

    // Add an entity to the manager
    void addEntity(std::unique_ptr<Entity> entity);

    // Get an entity by its unique ID; nullptr if not found
    Entity* getEntityById(const std::string& id) const;

    // Get children by parentId
    const std::vector<Entity*>* getChildren(const std::string& parentId) const;

    // Remove an entity by ID; returns true if removed
    bool removeEntity(const std::string& id);

    // Clear all entities
    void clear();

private:
    EntityManager() = default;
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Entity>> entities_;
    std::unordered_map<std::string, std::vector<Entity*>> childrenIndex_;
};
