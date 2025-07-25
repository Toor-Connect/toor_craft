#include "EntityManager.h"
#include <algorithm>

EntityManager &EntityManager::instance()
{
    static EntityManager manager;
    return manager;
}

void EntityManager::addEntity(std::unique_ptr<Entity> entity)
{
    const std::string &id = entity->getId();
    const std::string &parentId = entity->getParentId();

    // Insert into main storage
    entities_.emplace(id, std::move(entity));

    // Update parent-child index
    Entity *ptr = entities_[id].get();
    if (!parentId.empty())
    {
        childrenIndex_[parentId].push_back(ptr);
    }
}

Entity *EntityManager::getEntityById(const std::string &id) const
{
    auto it = entities_.find(id);
    if (it != entities_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

// Get children by parentId
const std::vector<Entity *> *EntityManager::getChildren(const std::string &parentId) const
{
    auto it = childrenIndex_.find(parentId);
    if (it != childrenIndex_.end())
    {
        return &(it->second);
    }
    return nullptr;
}

bool EntityManager::removeEntity(const std::string &id)
{
    auto it = entities_.find(id);
    if (it == entities_.end())
        return false;

    const std::string &parentId = it->second->getParentId();

    // Remove entity pointer from parent's children vector
    if (!parentId.empty())
    {
        auto childIt = childrenIndex_.find(parentId);
        if (childIt != childrenIndex_.end())
        {
            auto &siblings = childIt->second;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), it->second.get()), siblings.end());

            // Clean up empty vector to avoid clutter
            if (siblings.empty())
            {
                childrenIndex_.erase(childIt);
            }
        }
    }

    entities_.erase(it);
    return true;
}

void EntityManager::clear()
{
    entities_.clear();
    childrenIndex_.clear();
}

bool EntityManager::setFieldValue(const std::string& entityId, const std::string& fieldName, const std::string& value, std::string& error) {
    auto entity = getEntityById(entityId);
    if (!entity) {
        error = "Entity not found: " + entityId;
        return false;
    }
    return entity->setFieldValue(fieldName, value, error);
}

FieldValue* EntityManager::getFieldValue(const std::string& entityId, const std::string& fieldName) {
    auto entity = getEntityById(entityId);
    if (!entity) return nullptr;
    return entity->getFieldValue(fieldName);
}

bool EntityManager::validateEntity(const std::string& entityId, std::string& error) {
    auto entity = getEntityById(entityId);
    if (!entity) {
        error = "Entity not found: " + entityId;
        return false;
    }
    return entity->validate(error);
}
