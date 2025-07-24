#include "FieldValueFactory.h"

FieldValueFactory& FieldValueFactory::instance() {
    static FieldValueFactory factory;
    return factory;
}

FieldValueFactory::FieldValueFactory() = default;

void FieldValueFactory::registerType(const std::string& typeName, CreatorFunc creator) {
    creators_[typeName] = std::move(creator);
}

std::unique_ptr<FieldValue> FieldValueFactory::create(const std::string& typeName, const FieldSchema& schema) const {
    auto it = creators_.find(typeName);
    if (it == creators_.end()) {
        throw std::runtime_error("Unknown field value type: " + typeName);
    }
    return it->second(schema);
}
