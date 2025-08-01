#include <catch2/catch_test_macros.hpp>
#include "SchemaManager.h"
#include "EntityManager.h"
#include "EntitySchema.h"
#include "FieldValue.h"

TEST_CASE("EntityManager handles complex nested schema and multi-file data bundle")
{
  // --- Load complex schema ---
  std::unordered_map<std::string, std::string> schemas;
  schemas["smarthome.yaml"] = R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
fields:
  name:
    type: string
  location:
    type: object
    fields:
      city:
        type: string
      zipcode:
        type: integer
  tags:
    type: array
    element:
      type: string
)";

  schemas["device.yaml"] = R"(
entity_name: Device
children:
  sensors:
    entity: Sensor
fields:
  name:
    type: string
  power:
    type: boolean
  specs:
    type: object
    fields:
      manufacturer:
        type: string
      warranty_years:
        type: integer
)";

  schemas["sensor.yaml"] = R"(
entity_name: Sensor
fields:
  name:
    type: string
  readings:
    type: array
    element:
      type: object
      fields:
        timestamp:
          type: string
        value:
          type: float
)";

  SchemaManager &schemaMgr = SchemaManager::instance();
  schemaMgr.parseSchemaBundle(schemas);

  // --- Load multiple data files ---
  std::unordered_map<std::string, std::string> data;
  data["houses.yaml"] = R"(
house1:
  _schema: SmartHome
  name: My Smart Villa
  location:
    city: Barcelona
    zipcode: 8001
  tags:
    - modern
    - solar-powered
)";

  data["devices.yaml"] = R"(
device1:
  _schema: Device
  _parentid: house1
  name: Thermostat
  power: true
  specs:
    manufacturer: Nest
    warranty_years: 2

device2:
  _schema: Device
  _parentid: house1
  name: Air Purifier
  power: false
  specs:
    manufacturer: Dyson
    warranty_years: 3
)";

  data["sensors.yaml"] = R"(
sensor1:
  _schema: Sensor
  _parentid: device1
  name: Temperature Sensor
  readings:
    - timestamp: "2025-08-01T10:00:00Z"
      value: 23.5
    - timestamp: "2025-08-01T11:00:00Z"
      value: 24.1

sensor2:
  _schema: Sensor
  _parentid: device2
  name: Air Quality Sensor
  readings:
    - timestamp: "2025-08-01T10:15:00Z"
      value: 12.2
)";

  EntityManager &mgr = EntityManager::instance();
  mgr.clear();
  mgr.parseDataBundle(data);

  // --- Assertions ---

  SECTION("Root SmartHome entity is parsed correctly")
  {
    Entity *house = mgr.getEntityById("house1");
    REQUIRE(house != nullptr);
    REQUIRE(house->getSchema().getName() == "SmartHome");

    REQUIRE(house->getFieldValue("name")->toString() == "My Smart Villa");

    // ✅ Object field: location
    FieldValue *locVal = house->getFieldValue("location");
    REQUIRE(locVal != nullptr);
    auto dict = locVal->toString(); // Or check object subfields via API if implemented

    // ✅ Array field: tags
    FieldValue *tags = house->getFieldValue("tags");
    REQUIRE(tags != nullptr);
    REQUIRE(tags->toString().find("modern") != std::string::npos);
  }

  SECTION("Devices are linked under SmartHome")
  {
    const auto *children = mgr.getChildren("house1");
    REQUIRE(children != nullptr);
    REQUIRE(children->size() == 2);

    std::vector<std::string> ids;
    for (auto *child : *children)
      ids.push_back(child->getId());

    REQUIRE(std::find(ids.begin(), ids.end(), "device1") != ids.end());
    REQUIRE(std::find(ids.begin(), ids.end(), "device2") != ids.end());
  }

  SECTION("Sensors are linked under Devices")
  {
    const auto *device1Children = mgr.getChildren("device1");
    REQUIRE(device1Children != nullptr);
    REQUIRE(device1Children->size() == 1);
    REQUIRE((*device1Children)[0]->getId() == "sensor1");

    const auto *device2Children = mgr.getChildren("device2");
    REQUIRE(device2Children != nullptr);
    REQUIRE(device2Children->size() == 1);
    REQUIRE((*device2Children)[0]->getId() == "sensor2");
  }

  SECTION("Sensors have nested readings array parsed")
  {
    Entity *sensor1 = mgr.getEntityById("sensor1");
    REQUIRE(sensor1 != nullptr);

    FieldValue *readings = sensor1->getFieldValue("readings");
    REQUIRE(readings != nullptr);

    // This would typically be validated by iterating subvalues,
    // but we at least confirm it contains the timestamps
    REQUIRE(readings->toString().find("2025-08-01T10:00:00Z") != std::string::npos);
    REQUIRE(readings->toString().find("23.5") != std::string::npos);
  }
}
