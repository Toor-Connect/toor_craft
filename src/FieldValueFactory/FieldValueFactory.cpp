#include "FieldValueFactory.h"
#include "FieldSchemaFactory.h"

FieldValueFactory &FieldValueFactory::instance()
{
    static FieldValueFactory factory;
    return factory;
}

FieldValueFactory::FieldValueFactory()
{
    registerFieldValueType<StringFieldValue, StringFieldSchema>("string");
    registerFieldValueType<IntegerFieldValue, IntegerFieldSchema>("integer");
    registerFieldValueType<FloatFieldValue, FloatFieldSchema>("float");
    registerFieldValueType<BooleanFieldValue, BooleanFieldSchema>("boolean");
    registerFieldValueType<ReferenceFieldValue, ReferenceFieldSchema>("reference");
    registerFieldValueType<EnumFieldValue, EnumFieldSchema>("enum");
    registerFieldValueType<ArrayFieldValue, ArrayFieldSchema>("array");
    registerFieldValueType<ObjectFieldValue, ObjectFieldSchema>("object");
}

void FieldValueFactory::registerType(const std::string &typeName, CreatorFunc creator)
{
    creators_[typeName] = std::move(creator);
}

std::unique_ptr<FieldValue> FieldValueFactory::create(const std::string &typeName, const FieldSchema &schema) const
{
    auto it = creators_.find(typeName);
    if (it == creators_.end())
    {
        throw std::runtime_error("Unknown field value type: " + typeName);
    }
    return it->second(schema);
}
