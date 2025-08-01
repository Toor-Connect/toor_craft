#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include "FieldValue.h"
#include "FieldSchema.h"
#include "StringFieldValue.h"
#include "IntegerFieldValue.h"
#include "BooleanFieldValue.h"
#include "ReferenceFieldValue.h"
#include "EnumFieldValue.h"
#include "FloatFieldValue.h"
#include "ArrayFieldValue.h"
#include "ObjectFieldValue.h"

class FieldValueFactory
{
public:
    using CreatorFunc = std::function<std::unique_ptr<FieldValue>(const FieldSchema &)>;

    static FieldValueFactory &instance();

    void registerType(const std::string &typeName, CreatorFunc creator);

    std::unique_ptr<FieldValue> create(const std::string &typeName, const FieldSchema &schema) const;

    template <typename FieldValueType, typename FieldSchemaType>
    void registerFieldValueType(const std::string &typeName)
    {
        registerType(typeName, [typeName](const FieldSchema &schema)
                     {
            // Safe downcast to specific schema subclass
            auto derivedSchema = dynamic_cast<const FieldSchemaType*>(&schema);
            if (!derivedSchema)
            {
                throw std::runtime_error("Invalid schema type for " + typeName);
            }
            return std::make_unique<FieldValueType>(*derivedSchema); });
    }

private:
    std::unordered_map<std::string, CreatorFunc> creators_;

    FieldValueFactory();
    FieldValueFactory(const FieldValueFactory &) = delete;
    FieldValueFactory &operator=(const FieldValueFactory &) = delete;
};
