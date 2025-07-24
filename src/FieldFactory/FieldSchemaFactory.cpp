#include "FieldSchemaFactory.h"

FieldSchemaFactory& FieldSchemaFactory::instance() {
    static FieldSchemaFactory factory;
    return factory;
}

FieldSchemaFactory::FieldSchemaFactory() = default;

void FieldSchemaFactory::registerType(const std::string& typeName, CreatorFunc creator) {
    creators_[typeName] = std::move(creator);
}

std::unique_ptr<FieldSchema> FieldSchemaFactory::create(const std::string& typeName, const FieldSchemaConfig& config) const {
    auto it = creators_.find(typeName);
    if (it == creators_.end()) {
        throw std::runtime_error("Unknown field type: " + typeName);
    }
    return it->second(config);
}
