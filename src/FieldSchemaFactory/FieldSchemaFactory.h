#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <stdexcept>

#include "FieldSchema.h"
#include "StringFieldSchema.h"
#include "IntegerFieldSchema.h"
#include "BooleanFieldSchema.h"
#include "ReferenceFieldSchema.h"
#include "EnumFieldSchema.h"
#include "FloatFieldSchema.h"

class FieldSchemaFactory
{
public:
    using CreatorFunc = std::function<std::unique_ptr<FieldSchema>(const FieldSchemaConfig &)>;

    static FieldSchemaFactory &instance();

    void registerType(const std::string &typeName, CreatorFunc creator);

    std::unique_ptr<FieldSchema> create(const std::string &typeName, const FieldSchemaConfig &config) const;

    template <typename FieldType, typename ConfigType>
    void registerFieldSchemaType(const std::string &typeName)
    {
        registerType(typeName, [typeName](const FieldSchemaConfig &config)
                     {
            auto derivedConfig = dynamic_cast<const ConfigType*>(&config);
            if (!derivedConfig) {
                throw std::runtime_error("Invalid config type for " + typeName);
            }
            return std::make_unique<FieldType>(*derivedConfig); });
    }

private:
    std::unordered_map<std::string, CreatorFunc> creators_;

    FieldSchemaFactory();
    FieldSchemaFactory(const FieldSchemaFactory &) = delete;
    FieldSchemaFactory &operator=(const FieldSchemaFactory &) = delete;
};