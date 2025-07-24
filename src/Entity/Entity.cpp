#include "Entity.h"
#include "FieldValueFactory.h"  // Use FieldValueFactory, not FieldSchemaFactory

Entity::Entity(const EntitySchema& schema)
    : schema_(schema) 
{
    // Iterate over the fields of the schema
    for (const auto& [fieldName, fieldSchemaPtr] : schema_.getFields()) {
        // Create FieldValue using FieldValueFactory from the field schema
        auto fieldValue = FieldValueFactory::instance().create(fieldSchemaPtr->getTypeName(), *fieldSchemaPtr);
        fieldValues_.emplace(fieldName, std::move(fieldValue));
    }
}

const EntitySchema& Entity::getSchema() const {
    return schema_;
}

FieldValue* Entity::getFieldValue(const std::string& fieldName) {
    auto it = fieldValues_.find(fieldName);
    if (it != fieldValues_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool Entity::setFieldValue(const std::string& fieldName, const std::string& value, std::string& error) {
    auto* fieldValue = getFieldValue(fieldName);
    if (!fieldValue) {
        error = "Field not found: " + fieldName;
        return false;
    }
    return fieldValue->setValueFromString(value, error);
}

bool Entity::validate(std::string& error) const {
    for (const auto& [name, fieldValue] : fieldValues_) {
        if (!fieldValue->validate(error)) {
            return false;
        }
    }
    return true;
}

void Entity::setId(const std::string& id) {
    _id = id;
}

const std::string& Entity::getId() const {
    return _id;
}

void Entity::setParentId(const std::string& parentId) {
    _parentId = parentId;
}

const std::string& Entity::getParentId() const {
    return _parentId;
}
