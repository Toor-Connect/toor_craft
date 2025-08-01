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
    EntityManager::instance().setFieldValue(entityId, fieldName, value);
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
