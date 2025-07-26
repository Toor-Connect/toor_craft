#include "ReferenceFieldValue.h"
#include "EntityManager.h"

ReferenceFieldValue::ReferenceFieldValue(const ReferenceFieldSchema &schema)
    : FieldValue(schema), schema_(schema)
{
}

bool ReferenceFieldValue::setValueFromString(const std::string &value, std::string &error)
{
    // We expect 'value' to be an entity ID string
    // Optionally, validate that the referenced entity exists and matches the type
    auto entity = EntityManager::instance().getEntityById(value);
    if (!entity)
    {
        error = "Referenced entity with ID '" + value + "' does not exist";
        return false;
    }
    if (!schema_.getTargetEntityName().empty() &&
        entity->getSchema().getName() != schema_.getTargetEntityName())
    {
        error = "Referenced entity type mismatch, expected '" + schema_.getTargetEntityName() + "'";
        return false;
    }
    referencedId_ = value;
    return true;
}

bool ReferenceFieldValue::validate(std::string &error) const
{
    if (schema_.isRequired() && !referencedId_.has_value())
    {
        error = "Reference is required but no ID set";
        return false;
    }
    if (referencedId_)
    {
        auto entity = EntityManager::instance().getEntityById(*referencedId_);
        if (!entity)
        {
            error = "Referenced entity with ID '" + *referencedId_ + "' does not exist";
            return false;
        }
        if (!schema_.getTargetEntityName().empty() &&
            entity->getSchema().getName() != schema_.getTargetEntityName())
        {
            error = "Referenced entity type mismatch, expected '" + schema_.getTargetEntityName() + "'";
            return false;
        }
    }
    return true;
}

const std::optional<std::string> &ReferenceFieldValue::getReferencedId() const
{
    return referencedId_;
}

void ReferenceFieldValue::setReferencedId(const std::string &id)
{
    referencedId_ = id;
}

std::string ReferenceFieldValue::toString() const
{
    return referencedId_.value_or("");
}
