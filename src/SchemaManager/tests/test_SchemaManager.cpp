#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "SchemaManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"

TEST_CASE("SchemaManager handles a simple profile and entity")
{
    std::unordered_map<std::string, std::string> schemas;

    schemas["profile.yaml"] = R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
)";

    schemas["device.yaml"] = R"(
entity_name: Device
)";

    SchemaManager &mgr = SchemaManager::instance();
    mgr.parseSchemaBundle(schemas);

    REQUIRE(mgr.profileExists("SmartHome"));
    REQUIRE(mgr.getEntity("Device") != nullptr);

    auto childTags = mgr.getProfile("SmartHome")->getChildrenTags();
    REQUIRE(childTags.size() == 1);
    REQUIRE(childTags[0] == "devices");
}

TEST_CASE("SchemaManager handles all field types and extra params")
{
    std::unordered_map<std::string, std::string> schemas;

    schemas["profile.yaml"] = R"(
profile_name: FieldProfile
fields:
  - name: name
    type: string
    alias: full_name
  - name: active
    type: boolean
    required: true
  - name: age
    type: integer
    min: 0
    max: 120
  - name: temperature
    type: float
    min: -50.5
    max: 150.75
  - name: role
    type: enum
    values:
      - admin
      - user
      - guest
  - name: device_ref
    type: reference
    target: Device
children:
  devices:
    entity: Device
)";

    schemas["device.yaml"] = R"(
entity_name: Device
)";

    SchemaManager &mgr = SchemaManager::instance();
    mgr.parseSchemaBundle(schemas);

    auto profile = mgr.getProfile("FieldProfile");
    REQUIRE(profile != nullptr);

    SECTION("String field is parsed correctly")
    {
        auto field = profile->getField("name");
        REQUIRE(field != nullptr);

        auto stringField = dynamic_cast<const StringFieldSchema *>(field);
        REQUIRE(stringField != nullptr);
        REQUIRE(stringField->getName() == "name");
        REQUIRE(field->getAlias().has_value());
        REQUIRE(field->getAlias().value() == "full_name");
    }

    SECTION("Boolean field is parsed correctly")
    {
        auto field = profile->getField("active");
        REQUIRE(field != nullptr);

        auto boolField = dynamic_cast<const BooleanFieldSchema *>(field);
        REQUIRE(boolField != nullptr);
        REQUIRE(boolField->getName() == "active");
        REQUIRE(field->isRequired() == true);
    }

    SECTION("Integer field with min/max")
    {
        auto field = profile->getField("age");
        REQUIRE(field != nullptr);

        auto intField = dynamic_cast<const IntegerFieldSchema *>(field);
        REQUIRE(intField != nullptr);
        REQUIRE(intField->getName() == "age");
        REQUIRE(intField->getMinValue().has_value());
        REQUIRE(intField->getMaxValue().has_value());
        REQUIRE(intField->getMinValue().value() == 0);
        REQUIRE(intField->getMaxValue().value() == 120);
    }

    SECTION("Float field with min/max")
    {
        auto field = profile->getField("temperature");
        REQUIRE(field != nullptr);

        auto floatField = dynamic_cast<const FloatFieldSchema *>(field);
        REQUIRE(floatField != nullptr);
        REQUIRE(floatField->getName() == "temperature");
        REQUIRE(floatField->getMinValue().has_value());
        REQUIRE(floatField->getMaxValue().has_value());
        REQUIRE(floatField->getMinValue().value() == Catch::Approx(-50.5));
        REQUIRE(floatField->getMaxValue().value() == Catch::Approx(150.75));
    }

    SECTION("Enum field with values")
    {
        auto field = profile->getField("role");
        REQUIRE(field != nullptr);

        auto enumField = dynamic_cast<const EnumFieldSchema *>(field);
        REQUIRE(enumField != nullptr);
        REQUIRE(enumField->getName() == "role");
        REQUIRE(enumField->getAllowedValues().size() == 3);
        REQUIRE(enumField->getAllowedValues()[0] == "admin");
        REQUIRE(enumField->getAllowedValues()[1] == "user");
        REQUIRE(enumField->getAllowedValues()[2] == "guest");
    }

    SECTION("Reference field points to correct target")
    {
        auto field = profile->getField("device_ref");
        REQUIRE(field != nullptr);

        auto refField = dynamic_cast<const ReferenceFieldSchema *>(field);
        REQUIRE(refField != nullptr);
        REQUIRE(refField->getName() == "device_ref");
        REQUIRE(refField->getTargetEntityName() == "Device");
    }
}

TEST_CASE("SchemaManager throws when schema is invalid")
{
    std::unordered_map<std::string, std::string> badSchemas;

    badSchemas["broken.yaml"] = R"(
name: Invalid
)";

    SchemaManager &mgr = SchemaManager::instance();
    REQUIRE_THROWS_AS(mgr.parseSchemaBundle(badSchemas), std::runtime_error);

    std::unordered_map<std::string, std::string> danglingSchemas;
    danglingSchemas["profile.yaml"] = R"(
profile_name: BrokenProfile
children:
  missing_children:
    entity: MissingChild
)";
    REQUIRE_THROWS_AS(mgr.parseSchemaBundle(danglingSchemas), std::runtime_error);
}
