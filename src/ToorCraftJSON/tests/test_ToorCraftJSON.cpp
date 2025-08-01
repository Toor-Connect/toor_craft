#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "ToorCraftJSON.h"

using json = nlohmann::json;

TEST_CASE("ToorCraftJSON handles complex schemas, nested data, and JSON responses")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  // --- Complex Schema Setup ---
  std::unordered_map<std::string, std::string> schemas;
  schemas["home.yaml"] = R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
fields:
  name:
    type: string
    required: true
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
    required: true
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
    required: true
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

  SECTION("✅ Loading schemas works")
  {
    std::string resp = api.loadSchemas(schemas);
    auto parsed = json::parse(resp);
    REQUIRE(parsed["status"] == "ok");

    // ✅ Check schema listing
    std::string listResp = api.getSchemaList();
    auto list = json::parse(listResp);
    REQUIRE(list["status"] == "ok");
    REQUIRE(list["schemas"].size() == 3);
    REQUIRE(std::find(list["schemas"].begin(), list["schemas"].end(), "SmartHome") != list["schemas"].end());
  }

  // --- Data Setup ---
  std::unordered_map<std::string, std::string> data;
  data["homes.yaml"] = R"(
home1:
  _schema: SmartHome
  name: Villa Aurora
  address:
    city: Barcelona
    zipcode: 8001
  tags:
    - modern
    - solar-powered
)";

  data["devices.yaml"] = R"(
device1:
  _schema: Device
  _parentid: home1
  name: Thermostat
  active: true
  specs:
    manufacturer: Nest
    warranty_years: 2

device2:
  _schema: Device
  _parentid: home1
  name: Air Purifier
  active: false
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

  SECTION("✅ Loading data bundle works")
  {
    REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");
    REQUIRE(json::parse(api.loadData(data))["status"] == "ok");
  }

  SECTION("✅ Querying entity returns JSON")
  {
    api.loadSchemas(schemas);
    api.loadData(data);

    std::string entityResp = api.queryEntity("home1");
    auto parsed = json::parse(entityResp);

    REQUIRE(parsed["status"] == "ok");
    REQUIRE(parsed["entity"]["name"] == "Villa Aurora");
    // ✅ FIX: wrap OR expression to avoid Catch2 static assert
    REQUIRE((parsed["entity"]["address"].is_string() || parsed["entity"]["address"].is_object()));
  }

  SECTION("✅ setField updates values and validateEntity works")
  {
    api.loadSchemas(schemas);
    api.loadData(data);

    REQUIRE(json::parse(api.setField("device1", "name", "ThermoX"))["status"] == "ok");

    // ✅ Confirm update
    auto parsed = json::parse(api.queryEntity("device1"));
    REQUIRE(parsed["entity"]["name"] == "ThermoX");

    // ✅ Validate entity (should pass)
    REQUIRE(json::parse(api.validateEntity("device1"))["status"] == "ok");
  }

  SECTION("✅ getTree returns parent-child structure")
  {
    api.loadSchemas(schemas);
    api.loadData(data);

    auto treeResp = json::parse(api.getTree());
    REQUIRE(treeResp["status"] == "ok");
    REQUIRE(treeResp["tree"].is_array());

    // ✅ Find home1
    auto homeNode = std::find_if(treeResp["tree"].begin(), treeResp["tree"].end(),
                                 [](const json &n)
                                 { return n["id"] == "home1"; });
    REQUIRE(homeNode != treeResp["tree"].end());

    // ✅ Check that one of the children has id == "device1"
    auto childIt = std::find_if(homeNode->at("children").begin(),
                                homeNode->at("children").end(),
                                [](const json &child)
                                {
                                  return child["id"] == "device1";
                                });
    REQUIRE(childIt != homeNode->at("children").end());
  }
}

TEST_CASE("ToorCraftJSON handles error cases cleanly")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  SECTION("❌ Loading bad schema returns error JSON")
  {
    std::unordered_map<std::string, std::string> badSchemas;
    badSchemas["broken.yaml"] = R"(
fields:
  bad:
    type: string
)";
    auto resp = json::parse(api.loadSchemas(badSchemas));
    REQUIRE(resp["status"] == "error");
    REQUIRE(resp.contains("message"));
  }

  SECTION("❌ Loading data with unknown schema returns error JSON")
  {
    std::unordered_map<std::string, std::string> schemas;
    schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name: { type: string }
)";
    REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

    std::unordered_map<std::string, std::string> badData;
    badData["ghost.yaml"] = R"(
ghost1:
  _schema: Ghost
  name: Unknown
)";
    auto resp = json::parse(api.loadData(badData));
    REQUIRE(resp["status"] == "error");
  }

  SECTION("❌ setField on missing entity returns error JSON")
  {
    std::unordered_map<std::string, std::string> schemas;
    schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name: { type: string }
)";
    api.loadSchemas(schemas);

    auto resp = json::parse(api.setField("ghostDevice", "name", "test"));
    REQUIRE(resp["status"] == "error");
    REQUIRE(resp["message"].is_string());
  }

  SECTION("❌ validateEntity on missing entity returns error JSON")
  {
    auto resp = json::parse(api.validateEntity("ghostEntity"));
    REQUIRE(resp["status"] == "error");
  }
}
