#include <catch2/catch_test_macros.hpp>
#include "ToorCraftEngine.h"
#include "Entity.h"
#include "FieldValue.h"

TEST_CASE("ToorCraftEngine handles deeply nested schemas and data with throw-based API")
{
    ToorCraftEngine &engine = ToorCraftEngine::instance();

    // --- Step 1: Define a complex schema bundle ---
    std::unordered_map<std::string, std::string> schemas;
    schemas["home.yaml"] = R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
fields:
  name:
    type: string
  address:
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
  active:
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

    // ✅ loadSchemas should not throw
    REQUIRE_NOTHROW(engine.loadSchemas(schemas));

    // ✅ Verify schema list
    auto schemaList = engine.getSchemaList();
    REQUIRE(schemaList.size() == 3);
    REQUIRE(std::find(schemaList.begin(), schemaList.end(), "SmartHome") != schemaList.end());
    REQUIRE(std::find(schemaList.begin(), schemaList.end(), "Device") != schemaList.end());
    REQUIRE(std::find(schemaList.begin(), schemaList.end(), "Sensor") != schemaList.end());

    // --- Step 2: Define data bundle ---
    std::unordered_map<std::string, std::string> data;

    // Root SmartHome entity
    data["houses.yaml"] = R"(
house1:
  _schema: SmartHome
  name: Villa Aurora
  address:
    city: Barcelona
    zipcode: 8001
  tags:
    - modern
    - solar-powered
)";

    // Devices under SmartHome
    data["devices.yaml"] = R"(
device1:
  _schema: Device
  _parentid: house1
  name: Thermostat
  active: true
  specs:
    manufacturer: Nest
    warranty_years: 2

device2:
  _schema: Device
  _parentid: house1
  name: Air Purifier
  active: false
  specs:
    manufacturer: Dyson
    warranty_years: 3
)";

    // Sensors under devices
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

    // ✅ loadData should not throw
    REQUIRE_NOTHROW(engine.loadData(data));

    // --- Step 3: Query root entity ---
    Entity *house = engine.queryEntity("house1");
    REQUIRE(house != nullptr);
    REQUIRE(house->getSchema().getName() == "SmartHome");
    REQUIRE(house->getFieldValue("name")->toString() == "Villa Aurora");

    // --- Step 4: Query child device entity ---
    Entity *device = engine.queryEntity("device1");
    REQUIRE(device != nullptr);
    REQUIRE(device->getFieldValue("name")->toString() == "Thermostat");
    REQUIRE(device->getFieldValue("active")->toString() == "true");

    // --- Step 5: Update a field using engine.setField (now throws on error) ---
    REQUIRE_NOTHROW(engine.setField("device1", "name", "ThermoX"));
    REQUIRE(device->getFieldValue("name")->toString() == "ThermoX");

    // --- Step 6: Validate entities (throws if validation fails) ---
    REQUIRE_NOTHROW(engine.validateEntity("device1"));

    // --- Step 7: Inspect tree structure ---
    auto parents = engine.getParents();
    REQUIRE(parents.size() == 1); // house1 should be root
    REQUIRE(parents[0]->getId() == "house1");

    // ✅ Device children under SmartHome
    const auto *deviceChildren = engine.getChildren("house1");
    REQUIRE(deviceChildren != nullptr);
    REQUIRE(deviceChildren->size() == 2);

    std::vector<std::string> childIds;
    for (auto *child : *deviceChildren)
    {
        childIds.push_back(child->getId());
    }
    REQUIRE(std::find(childIds.begin(), childIds.end(), "device1") != childIds.end());
    REQUIRE(std::find(childIds.begin(), childIds.end(), "device2") != childIds.end());

    // --- Step 8: Check nested array object values on sensor ---
    Entity *sensor1 = engine.queryEntity("sensor1");
    REQUIRE(sensor1 != nullptr);

    FieldValue *readings = sensor1->getFieldValue("readings");
    REQUIRE(readings != nullptr);
    // readings should serialize to a string that contains timestamps and values
    REQUIRE(readings->toString().find("2025-08-01T10:00:00Z") != std::string::npos);
    REQUIRE(readings->toString().find("23.5") != std::string::npos);
}
