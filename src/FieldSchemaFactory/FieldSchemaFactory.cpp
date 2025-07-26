#include "FieldSchemaFactory.h"

FieldSchemaFactory &FieldSchemaFactory::instance()
{
    static FieldSchemaFactory factory;
    return factory;
}

FieldSchemaFactory::FieldSchemaFactory()
{
    registerFieldSchemaType<StringFieldSchema, StringFieldSchemaConfig>("string");
    registerFieldSchemaType<IntegerFieldSchema, IntegerFieldSchemaConfig>("integer");
    registerFieldSchemaType<FloatFieldSchema, FloatFieldSchemaConfig>("float");
    registerFieldSchemaType<BooleanFieldSchema, BooleanFieldSchemaConfig>("boolean");
    registerFieldSchemaType<ReferenceFieldSchema, ReferenceFieldSchemaConfig>("reference");
    registerFieldSchemaType<EnumFieldSchema, EnumFieldSchemaConfig>("enum");
}

void FieldSchemaFactory::registerType(const std::string &typeName, CreatorFunc creator)
{
    creators_[typeName] = std::move(creator);
}

std::unique_ptr<FieldSchema> FieldSchemaFactory::create(const std::string &typeName, const FieldSchemaConfig &config) const
{
    auto it = creators_.find(typeName);
    if (it == creators_.end())
    {
        throw std::runtime_error("Unknown field type: " + typeName);
    }
    return it->second(config);
}