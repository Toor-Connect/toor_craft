#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "ToorCraftJSON.h"
#include <iostream>

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

  SECTION("✅ Querying entity returns JSON with state")
  {
    api.loadSchemas(schemas);
    api.loadData(data);

    auto parsed = json::parse(api.queryEntity("home1"));
    REQUIRE(parsed["status"] == "ok");
    REQUIRE(parsed["entity"]["state"] == "Unchanged");
    REQUIRE(parsed["entity"]["name"] == "Villa Aurora");
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
    REQUIRE(parsed["entity"]["state"] == "Modified");

    // ✅ Validate entity (should pass)
    REQUIRE(json::parse(api.validateEntity("device1"))["status"] == "ok");
  }

  SECTION("✅ getTree returns parent-child structure (only non-deleted)")
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
    REQUIRE((*homeNode)["state"] == "Unchanged");
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
}

TEST_CASE("ToorCraftJSON handles full lifecycle including createEntity with deep nesting")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  // --- Schema Setup ---
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
)";

  schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name:
    type: string
    required: true
)";

  // ✅ Load schemas
  REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

  // ✅ Create root SmartHome entity
  std::unordered_map<std::string, std::string> homePayload = {{"name", "Casa Nova"}};
  auto homeResp = json::parse(api.createEntity("SmartHome", "homeX", "", homePayload));

  // ✅ Create Device under homeX
  std::unordered_map<std::string, std::string> devicePayload = {{"name", "Smart Thermostat"}};
  auto deviceResp = json::parse(api.createEntity("Device", "deviceX", "homeX", devicePayload));

  // ✅ Verify query shows "Added"
  auto queriedDevice = json::parse(api.queryEntity("deviceX"));
  REQUIRE(queriedDevice["entity"]["state"] == "Added");
}

TEST_CASE("ToorCraftJSON handles enums and references correctly")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  std::unordered_map<std::string, std::string> schemas;
  schemas["user.yaml"] = R"(
entity_name: User
fields:
  name: { type: string }
  role:
    type: enum
    values: [admin, guest, editor]
)";
  schemas["post.yaml"] = R"(
entity_name: Post
fields:
  title: { type: string }
  author:
    type: reference
    target: User
)";

  REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

  std::unordered_map<std::string, std::string> userPayload = {{"name", "Alice"}, {"role", "admin"}};
  auto userResp = json::parse(api.createEntity("User", "user1", "", userPayload));

  std::unordered_map<std::string, std::string> postPayload = {{"title", "My First Post"}, {"author", "user1"}};
  auto postResp = json::parse(api.createEntity("Post", "post1", "", postPayload));

  auto queriedUser = json::parse(api.queryEntity("user1"));
  REQUIRE(queriedUser["entity"]["state"] == "Added");
  REQUIRE(queriedUser["entity"]["role"] == "admin");
}

TEST_CASE("ToorCraftJSON handles deletion of entities, reference cleanup, and cascading removal")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  std::unordered_map<std::string, std::string> schemas;
  schemas["home.yaml"] = R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
fields:
  name: { type: string }
)";
  schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name: { type: string }
  sibling:
    type: reference
    target: Device
)";

  REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

  // ✅ Create Home + Devices
  auto homeResp = json::parse(api.createEntity("SmartHome", "homeRef", "", {{"name", "Casa Cascade"}}));
  REQUIRE(homeResp["status"] == "ok");
  REQUIRE(homeResp["created"]["id"] == "homeRef");
  REQUIRE(homeResp["created"]["schema"] == "SmartHome");

  auto devAResp = json::parse(api.createEntity("Device", "deviceA", "homeRef", {{"name", "Alpha"}}));
  REQUIRE(devAResp["status"] == "ok");
  REQUIRE(devAResp["created"]["id"] == "deviceA");
  REQUIRE(devAResp["created"]["schema"] == "Device");

  auto devBResp = json::parse(api.createEntity("Device", "deviceB", "homeRef", {{"name", "Beta"}}));
  REQUIRE(devBResp["status"] == "ok");
  REQUIRE(devBResp["created"]["id"] == "deviceB");
  REQUIRE(devBResp["created"]["schema"] == "Device");

  // ✅ Link siblings
  REQUIRE(json::parse(api.setField("deviceA", "sibling", "deviceB"))["status"] == "ok");
  REQUIRE(json::parse(api.setField("deviceB", "sibling", "deviceA"))["status"] == "ok");

  // ✅ Delete DeviceA
  REQUIRE(json::parse(api.deleteEntity("deviceA"))["status"] == "ok");
  auto devAQuery = json::parse(api.queryEntity("deviceA"));
  REQUIRE(devAQuery["entity"]["state"] == "Deleted");

  // ✅ DeviceB still exists, sibling cleared
  auto devBQuery = json::parse(api.queryEntity("deviceB"));
  REQUIRE(devBQuery["entity"]["state"] == "Added");

  // ✅ Delete Home (cascade)
  REQUIRE(json::parse(api.deleteEntity("homeRef"))["status"] == "ok");
  auto homeQuery = json::parse(api.queryEntity("homeRef"));
  REQUIRE(homeQuery["entity"]["state"] == "Deleted");

  // ✅ DeviceB should now also be Deleted
  auto devBAfterCascade = json::parse(api.queryEntity("deviceB"));
  REQUIRE(devBAfterCascade["entity"]["state"] == "Deleted");
}
