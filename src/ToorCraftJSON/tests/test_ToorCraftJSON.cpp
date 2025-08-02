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
TEST_CASE("ToorCraftJSON handles full lifecycle including createEntity with deep nesting")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  // --- 🏗 Complex Schema Setup with 3-level nesting ---
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
      geo:
        type: object
        fields:
          lat:
            type: float
          lng:
            type: float
  tags:
    type: array
    element:
      type: object
      fields:
        label:
          type: string
        meta:
          type: object
          fields:
            color:
              type: string
            priority:
              type: integer
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
  ports:
    type: array
    element:
      type: object
      fields:
        number:
          type: integer
        protocols:
          type: array
          element:
            type: string
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
        metrics:
          type: object
          fields:
            value:
              type: float
            unit:
              type: string
)";

  // ✅ Load schemas first
  REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

  // ✅ Create root SmartHome entity with nested JSON
  std::unordered_map<std::string, std::string> homePayload = {
      {"name", "Casa Nova"},
      {"address", R"({"city": "Madrid", "zipcode": 28001, "geo": {"lat": 40.4168, "lng": -3.7038}})"},
      {"tags", R"([
        {"label": "modern", "meta": {"color": "blue", "priority": 1}},
        {"label": "eco", "meta": {"color": "green", "priority": 2}}
      ])"}};

  std::string homeResp = api.createEntity("SmartHome", "homeX", "", homePayload);
  auto homeParsed = json::parse(homeResp);
  REQUIRE(homeParsed["status"] == "ok");
  REQUIRE(homeParsed["created"]["id"] == "homeX");
  REQUIRE(homeParsed["created"]["schema"] == "SmartHome");
  REQUIRE(homeParsed["created"]["parentId"].is_null());

  // ✅ Verify homeX appears in getRoot
  auto rootParsed = json::parse(api.getRoot());
  REQUIRE(rootParsed["status"] == "ok");
  auto rootIt = std::find_if(rootParsed["root"].begin(), rootParsed["root"].end(),
                             [](const json &node)
                             { return node["id"] == "homeX"; });
  REQUIRE(rootIt != rootParsed["root"].end());

  // ✅ Create Device under homeX with nested object + array of objects
  std::unordered_map<std::string, std::string> devicePayload = {
      {"name", "Smart Thermostat"},
      {"active", "true"},
      {"specs", R"({"manufacturer": "Nest", "warranty_years": 3})"},
      {"ports", R"([
        {"number": 1, "protocols": ["Zigbee", "WiFi"]},
        {"number": 2, "protocols": ["Bluetooth"]}
      ])"}};

  std::string deviceResp = api.createEntity("Device", "deviceX", "homeX", devicePayload);
  auto deviceParsed = json::parse(deviceResp);
  REQUIRE(deviceParsed["status"] == "ok");
  REQUIRE(deviceParsed["created"]["id"] == "deviceX");
  REQUIRE(deviceParsed["created"]["schema"] == "Device");
  REQUIRE(deviceParsed["created"]["parentId"] == "homeX");

  // ✅ Verify deviceX appears as a child of homeX in tree
  auto treeParsed = json::parse(api.getTree());
  REQUIRE(treeParsed["status"] == "ok");

  auto homeNode = std::find_if(treeParsed["tree"].begin(), treeParsed["tree"].end(),
                               [](const json &n)
                               { return n["id"] == "homeX"; });
  REQUIRE(homeNode != treeParsed["tree"].end());

  auto childIt = std::find_if(homeNode->at("children").begin(),
                              homeNode->at("children").end(),
                              [](const json &child)
                              { return child["id"] == "deviceX"; });
  REQUIRE(childIt != homeNode->at("children").end());

  // ✅ Query the device and check its fields
  auto queriedDevice = json::parse(api.queryEntity("deviceX"));
  REQUIRE(queriedDevice["status"] == "ok");
  REQUIRE(queriedDevice["entity"]["name"] == "Smart Thermostat");

  // ✅ Check nested specs object
  REQUIRE(queriedDevice["entity"]["specs"].is_object());
  REQUIRE(queriedDevice["entity"]["specs"]["manufacturer"] == "Nest");

  // ✅ Check deeply nested array-of-objects (ports)
  REQUIRE(queriedDevice["entity"]["ports"].is_array());
  REQUIRE(queriedDevice["entity"]["ports"][0]["protocols"].is_array());
  REQUIRE(queriedDevice["entity"]["ports"][0]["protocols"][0] == "Zigbee");
}

TEST_CASE("ToorCraftJSON handles enums and references correctly")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  // --- 🏗 Schema with ENUM and REFERENCE ---
  std::unordered_map<std::string, std::string> schemas;
  schemas["user.yaml"] = R"(
entity_name: User
fields:
  name:
    type: string
  role:
    type: enum
    values: [admin, guest, editor]
)";

  schemas["post.yaml"] = R"(
entity_name: Post
fields:
  title:
    type: string
  author:
    type: reference
    target: User
)";

  // ✅ Load schemas
  REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

  // ✅ Create a user with enum role
  std::unordered_map<std::string, std::string> userPayload = {
      {"name", "Alice"},
      {"role", "admin"}};

  std::string userResp = api.createEntity("User", "user1", "", userPayload);
  auto userParsed = json::parse(userResp);
  REQUIRE(userParsed["status"] == "ok");
  REQUIRE(userParsed["created"]["id"] == "user1");
  REQUIRE(userParsed["created"]["schema"] == "User");

  // ✅ Create a post that REFERENCES the user
  std::unordered_map<std::string, std::string> postPayload = {
      {"title", "My First Post"},
      {"author", "user1"}};

  std::string postResp = api.createEntity("Post", "post1", "", postPayload);
  auto postParsed = json::parse(postResp);
  REQUIRE(postParsed["status"] == "ok");
  REQUIRE(postParsed["created"]["id"] == "post1");
  REQUIRE(postParsed["created"]["schema"] == "Post");

  // ✅ Query the user and verify ENUM field serialization
  auto queriedUser = json::parse(api.queryEntity("user1"));
  REQUIRE(queriedUser["status"] == "ok");
  REQUIRE(queriedUser["entity"]["role"] == "admin");

  // ✅ Query the post and verify REFERENCE serialization
  auto queriedPost = json::parse(api.queryEntity("post1"));
  REQUIRE(queriedPost["status"] == "ok");
  REQUIRE(queriedPost["entity"]["author"] == "user1");
}

TEST_CASE("ToorCraftJSON handles deletion of entities, reference cleanup, and cascading removal")
{
  ToorCraftJSON &api = ToorCraftJSON::instance();

  // --- 🏗 Schema Setup ---
  std::unordered_map<std::string, std::string> schemas;
  schemas["home.yaml"] = R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
fields:
  name:
    type: string
)";

  schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name:
    type: string
  sibling:
    type: reference
    target: Device
)";

  // ✅ Load schemas
  REQUIRE(json::parse(api.loadSchemas(schemas))["status"] == "ok");

  // ✅ Create Home
  std::unordered_map<std::string, std::string> homePayload = {
      {"name", "Casa Cascade"}};
  REQUIRE(json::parse(api.createEntity("SmartHome", "homeRef", "", homePayload))["status"] == "ok");

  // ✅ Create Device1 under Home
  std::unordered_map<std::string, std::string> dev1Payload = {
      {"name", "Device Alpha"}};
  REQUIRE(json::parse(api.createEntity("Device", "deviceA", "homeRef", dev1Payload))["status"] == "ok");

  // ✅ Create Device2 under Home
  std::unordered_map<std::string, std::string> dev2Payload = {
      {"name", "Device Beta"}};
  REQUIRE(json::parse(api.createEntity("Device", "deviceB", "homeRef", dev2Payload))["status"] == "ok");

  // ✅ Set cross-references
  REQUIRE(json::parse(api.setField("deviceA", "sibling", "deviceB"))["status"] == "ok");
  REQUIRE(json::parse(api.setField("deviceB", "sibling", "deviceA"))["status"] == "ok");

  // 🔍 Confirm references before deletion
  auto devA = json::parse(api.queryEntity("deviceA"));
  auto devB = json::parse(api.queryEntity("deviceB"));
  REQUIRE(devA["entity"]["sibling"] == "deviceB");
  REQUIRE(devB["entity"]["sibling"] == "deviceA");

  // ✅ Delete DeviceA
  auto deleteA = json::parse(api.deleteEntity("deviceA"));
  REQUIRE(deleteA["status"] == "ok");

  // 🔍 Query DeviceA should return not_found
  auto devAQuery = json::parse(api.queryEntity("deviceA"));
  REQUIRE(devAQuery["status"] == "not_found");

  // 🔍 DeviceB should now have sibling cleared (null or not present)
  auto devBAfter = json::parse(api.queryEntity("deviceB"));
  REQUIRE(devBAfter["status"] == "ok");
  REQUIRE((!devBAfter["entity"].contains("sibling") || devBAfter["entity"]["sibling"].is_null()));

  // ✅ Delete Home
  auto deleteHome = json::parse(api.deleteEntity("homeRef"));
  REQUIRE(deleteHome["status"] == "ok");

  // 🔍 DeviceB should also be gone (cascade delete)
  auto devBQuery = json::parse(api.queryEntity("deviceB"));
  REQUIRE(devBQuery["status"] == "not_found");

  // 🔍 Finally, check tree is now empty
  auto treeAfter = json::parse(api.getTree());
  REQUIRE(treeAfter["tree"].empty());
}
