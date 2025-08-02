#include "ToorCraftEngine.h"
#include "SchemaManager.h"
#include "EntityManager.h"
#include "Entity.h"

ToorCraftEngine &ToorCraftEngine::instance()
{
    static ToorCraftEngine inst;
    return inst;
}

void ToorCraftEngine::loadSchemas(const std::unordered_map<std::string, std::string> &schemas)
{
    SchemaManager::instance().parseSchemaBundle(schemas);
}

std::vector<std::string> ToorCraftEngine::getSchemaList() const
{
    return SchemaManager::instance().getEntitySchemaNames();
}

void ToorCraftEngine::loadData(const std::unordered_map<std::string, std::string> &data)
{
    EntityManager::instance().parseDataBundle(data);
}

Entity *ToorCraftEngine::queryEntity(const std::string &id) const
{
    return EntityManager::instance().getEntityById(id);
}

void ToorCraftEngine::setField(const std::string &entityId,
                               const std::string &fieldName,
                               const std::string &value)
{
    auto &manager = EntityManager::instance();

    Entity *entity = manager.getEntityById(entityId);
    if (!entity)
    {
        throw std::runtime_error("Entity not found: " + entityId);
    }

    if (entity->getState() == EntityState::Deleted)
    {
        throw std::runtime_error("Cannot update field on a deleted entity: " + entityId);
    }

    manager.setFieldValue(entityId, fieldName, value);

    if (entity->getState() != EntityState::Added)
    {
        entity->setState(EntityState::Modified);
    }
}

void ToorCraftEngine::validateEntity(const std::string &entityId)
{
    EntityManager::instance().validate(entityId);
}

std::vector<Entity *> ToorCraftEngine::getParents() const
{
    return EntityManager::instance().getParents();
}

const std::vector<Entity *> *ToorCraftEngine::getChildren(const std::string &parentId) const
{
    return EntityManager::instance().getChildren(parentId);
}

const EntitySchema *ToorCraftEngine::getSchema(const std::string &name) const
{
    EntitySchema *schema = SchemaManager::instance().getEntitySchema(name);
    if (!schema)
    {
        throw std::runtime_error("Schema '" + name + "' not found");
    }
    return schema;
}

std::string ToorCraftEngine::getParent(const std::string &entityId) const
{
    Entity *ent = EntityManager::instance().getEntityById(entityId);
    if (!ent)
        throw std::runtime_error("Entity not found: " + entityId);

    return ent->getParentId();
}

void ToorCraftEngine::createEntity(const std::string &schemaName,
                                   const std::string &entityId,
                                   const std::string &parentId,
                                   const std::unordered_map<std::string, std::string> &fieldData)
{
    EntitySchema *schema = SchemaManager::instance().getEntitySchema(schemaName);
    if (!schema)
        throw std::runtime_error("Schema not found: " + schemaName);

    auto newEntity = std::make_unique<Entity>(*schema);
    newEntity->setId(entityId);

    if (!parentId.empty())
        newEntity->setParentId(parentId);

    for (const auto &[fname, fval] : fieldData)
    {
        try
        {
            newEntity->setFieldValue(fname, fval);
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Error setting field '" + fname + "': " + e.what());
        }
    }

    Entity *rawPtr = newEntity.get();
    newEntity->setState(EntityState::Added);
    EntityManager::instance().addEntity(std::move(newEntity));
}

void ToorCraftEngine::deleteEntity(const std::string &entityId)
{
    try
    {
        EntityManager &mgr = EntityManager::instance();

        Entity *entity = mgr.getEntityById(entityId);
        if (!entity)
        {
            throw std::runtime_error("Entity '" + entityId + "' not found");
        }

        if (!mgr.removeEntity(entityId))
        {
            throw std::runtime_error("Failed to remove entity '" + entityId + "'");
        }
    }
    catch (const std::exception &ex)
    {
        throw std::runtime_error(std::string("deleteEntity failed: ") + ex.what());
    }
}
