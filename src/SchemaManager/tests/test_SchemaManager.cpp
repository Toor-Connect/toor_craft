#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "SchemaManager.h"
#include "EntitySchema.h"
#include "FieldSchemaFactory.h"
#include "IntegerFieldSchema.h"
#include "FloatFieldSchema.h"
#include "EnumFieldSchema.h"
#include "BooleanFieldSchema.h"
#include "ReferenceFieldSchema.h"
#include "StringFieldSchema.h"
#include "ObjectFieldSchema.h"
#include "ArrayFieldSchema.h"

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

TEST_CASE("SchemaManager handles all field types including object and array")
{
  std::unordered_map<std::string, std::string> schemas;

  schemas["profile.yaml"] = R"(
profile_name: FieldProfile
fields:
  name:
    type: string
    alias: full_name
  active:
    type: boolean
    required: true
  age:
    type: integer
    min: 0
    max: 120
  temperature:
    type: float
    min: -50.5
    max: 150.75
  role:
    type: enum
    values:
      - admin
      - user
      - guest
  device_ref:
    type: reference
    target: Device
  settings:
    type: object
    fields:
      volume:
        type: integer
        min: 0
        max: 100
      mode:
        type: string
  tags:
    type: array
    element:
      type: string
  sensors:
    type: array
    element:
      type: object
      fields:
        id:
          type: string
        value:
          type: float
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

  SECTION("Object field is parsed correctly")
  {
    auto field = profile->getField("settings");
    REQUIRE(field != nullptr);

    auto objField = dynamic_cast<const ObjectFieldSchema *>(field);
    REQUIRE(objField != nullptr);
    REQUIRE(objField->getName() == "settings");

    // ✅ Check inner fields
    auto innerField = objField->getField("volume");
    REQUIRE(innerField != nullptr);
    REQUIRE(dynamic_cast<const IntegerFieldSchema *>(innerField) != nullptr);
  }

  SECTION("Array of strings is parsed correctly")
  {
    auto field = profile->getField("tags");
    REQUIRE(field != nullptr);

    auto arrField = dynamic_cast<const ArrayFieldSchema *>(field);
    REQUIRE(arrField != nullptr);
    REQUIRE(arrField->getName() == "tags");

    // ✅ Verify element type is string
    auto &elemSchema = arrField->getElementSchema();
    REQUIRE(elemSchema.getTypeName() == "string");
  }

  SECTION("Array of objects is parsed correctly")
  {
    auto field = profile->getField("sensors");
    REQUIRE(field != nullptr);

    auto arrField = dynamic_cast<const ArrayFieldSchema *>(field);
    REQUIRE(arrField != nullptr);
    REQUIRE(arrField->getName() == "sensors");

    // ✅ Element should be an object field
    auto &elemSchema = arrField->getElementSchema();
    REQUIRE(elemSchema.getTypeName() == "object");

    const ObjectFieldSchema *objElem = dynamic_cast<const ObjectFieldSchema *>(&elemSchema);
    REQUIRE(objElem != nullptr);

    // ✅ Check that object fields exist inside the array element
    REQUIRE(objElem->getField("id") != nullptr);
    REQUIRE(objElem->getField("value") != nullptr);
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

TEST_CASE("FieldSchemaFactory direct creation with rvalues")
{
  FieldSchemaFactory &factory = FieldSchemaFactory::instance();

  SECTION("IntegerFieldSchema can be created with moved config")
  {
    IntegerFieldSchemaConfig cfg;
    cfg.name = "intTest";
    cfg.minValue = 1;
    cfg.maxValue = 99;

    auto field = factory.create("integer", std::move(cfg));

    auto intField = dynamic_cast<IntegerFieldSchema *>(field.get());
    REQUIRE(intField != nullptr);
    REQUIRE(intField->getName() == "intTest");
    REQUIRE(intField->getMinValue().has_value());
    REQUIRE(intField->getMaxValue().has_value());
  }
}

TEST_CASE("SchemaManager handles deeply nested arrays and objects")
{
  std::unordered_map<std::string, std::string> schemas;

  schemas["profile.yaml"] = R"(
profile_name: DeepNestProfile
fields:
  complex_array:
    type: array
    element:
      type: object
      fields:
        level1_name:
          type: string
        level1_array:
          type: array
          element:
            type: object
            fields:
              level2_id:
                type: integer
              level2_tags:
                type: array
                element:
                  type: string
)";

  SchemaManager &mgr = SchemaManager::instance();
  mgr.parseSchemaBundle(schemas);

  auto profile = mgr.getProfile("DeepNestProfile");
  REQUIRE(profile != nullptr);

  // ✅ Check the top-level array
  auto complexArray = profile->getField("complex_array");
  REQUIRE(complexArray != nullptr);
  auto arrField = dynamic_cast<const ArrayFieldSchema *>(complexArray);
  REQUIRE(arrField != nullptr);

  // ✅ Array element must be an object
  auto &level1Schema = arrField->getElementSchema();
  REQUIRE(level1Schema.getTypeName() == "object");

  auto level1Obj = dynamic_cast<const ObjectFieldSchema *>(&level1Schema);
  REQUIRE(level1Obj != nullptr);

  // ✅ Object must have "level1_name" (string) and "level1_array" (array)
  auto level1Name = level1Obj->getField("level1_name");
  REQUIRE(level1Name != nullptr);
  REQUIRE(level1Name->getTypeName() == "string");

  auto level1ArrayField = level1Obj->getField("level1_array");
  REQUIRE(level1ArrayField != nullptr);
  auto level1Array = dynamic_cast<const ArrayFieldSchema *>(level1ArrayField);
  REQUIRE(level1Array != nullptr);

  // ✅ Dive into array of objects (Level 2)
  auto &level2Schema = level1Array->getElementSchema();
  REQUIRE(level2Schema.getTypeName() == "object");

  auto level2Obj = dynamic_cast<const ObjectFieldSchema *>(&level2Schema);
  REQUIRE(level2Obj != nullptr);

  // ✅ Level 2 object must have integer + array of strings
  auto level2Id = level2Obj->getField("level2_id");
  REQUIRE(level2Id != nullptr);
  REQUIRE(level2Id->getTypeName() == "integer");

  auto level2TagsField = level2Obj->getField("level2_tags");
  REQUIRE(level2TagsField != nullptr);
  auto level2TagsArray = dynamic_cast<const ArrayFieldSchema *>(level2TagsField);
  REQUIRE(level2TagsArray != nullptr);

  // ✅ Final check: array element type should be string
  auto &tagElem = level2TagsArray->getElementSchema();
  REQUIRE(tagElem.getTypeName() == "string");
}
