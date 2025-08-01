#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Entity.h"

class EntityManager;

class IEntityQuery
{
public:
    virtual ~IEntityQuery() = default;
    virtual std::vector<Entity *> execute(const EntityManager &manager) const = 0;
};

class EntityManager
{
public:
    static EntityManager &instance();

    void parseDataBundle(const std::unordered_map<std::string, std::string> &bundleContent);
    void addEntity(std::unique_ptr<Entity> entity);
    Entity *getEntityById(const std::string &id) const;
    bool removeEntity(const std::string &id);
    void clear();

    void setFieldValue(const std::string &entityId,
                       const std::string &fieldName,
                       const std::string &value);

    FieldValue *getFieldValue(const std::string &entityId, const std::string &fieldName);

    void validate(const std::string &entityId);

    std::vector<Entity *> query(const IEntityQuery &query) const;
    const std::vector<Entity *> &getParents() const;
    const std::vector<Entity *> *getChildren(const std::string &parentId) const;

private:
    EntityManager() = default;
    EntityManager(const EntityManager &) = delete;
    EntityManager &operator=(const EntityManager &) = delete;

    std::vector<Entity *> parents_;
    std::unordered_map<std::string, std::unique_ptr<Entity>> entities_;
    std::unordered_map<std::string, std::vector<Entity *>> childrenIndex_;
};
