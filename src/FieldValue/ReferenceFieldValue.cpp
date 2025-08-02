#include "ReferenceFieldValue.h"
#include "EntityManager.h"
#include <nlohmann/json.hpp>

ReferenceFieldValue::ReferenceFieldValue(const ReferenceFieldSchema &schema)
    : FieldValue(schema), schema_(schema)
{
}

void ReferenceFieldValue::setValueFromString(const std::string &value)
{
    auto entity = EntityManager::instance().getEntityById(value);
    if (!entity)
    {
        throw std::runtime_error("Referenced entity with ID '" + value + "' does not exist");
    }

    if (!schema_.getTargetEntityName().empty() &&
        entity->getSchema().getName() != schema_.getTargetEntityName())
    {
        throw std::runtime_error(
            "Referenced entity type mismatch, expected '" + schema_.getTargetEntityName() + "'");
    }

    referencedId_ = value;
}

void ReferenceFieldValue::validate() const
{
    if (schema_.isRequired() && !referencedId_.has_value())
    {
        throw std::runtime_error("Reference is required but no ID set");
    }

    if (referencedId_)
    {
        auto entity = EntityManager::instance().getEntityById(*referencedId_);
        if (!entity)
        {
            throw std::runtime_error("Referenced entity with ID '" + *referencedId_ + "' does not exist");
        }

        if (!schema_.getTargetEntityName().empty() &&
            entity->getSchema().getName() != schema_.getTargetEntityName())
        {
            throw std::runtime_error(
                "Referenced entity type mismatch, expected '" + schema_.getTargetEntityName() + "'");
        }
    }
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

bool ReferenceFieldValue::isEmpty() const
{
    return !referencedId_.has_value();
}

std::string ReferenceFieldValue::toJson() const
{
    nlohmann::json j;
    if (referencedId_.has_value() && !referencedId_->empty())
    {
        j = *referencedId_;
    }
    else
    {
        j = nullptr;
    }
    return j.dump();
}
