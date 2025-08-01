#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "ToorCraftRouter.h"

using json = nlohmann::json;

TEST_CASE("ToorCraftRouter handles full lifecycle commands and all API calls")
{
  auto &router = ToorCraftRouter::instance();

  // --- 1️⃣ Load schemas ---
  json schemaReq = {
      {"command", "loadSchemas"},
      {"schemas", {{"home.yaml", R"(
profile_name: SmartHome
children:
  devices:
    entity: Device
fields:
  name:
    type: string
    required: true
)"},
                   {"device.yaml", R"(
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
)"},
                   {"sensor.yaml", R"(
entity_name: Sensor
fields:
  name:
    type: string
    required: true
)"}}}};

  auto schemaResp = json::parse(router.handleRequest(schemaReq.dump()));
  REQUIRE(schemaResp["status"] == "ok");

  // --- 2️⃣ getSchemaList ---
  json listReq = {{"command", "getSchemaList"}};
  auto listResp = json::parse(router.handleRequest(listReq.dump()));
  REQUIRE(listResp["status"] == "ok");
  REQUIRE(listResp["schemas"].is_array());
  REQUIRE(std::find(listResp["schemas"].begin(), listResp["schemas"].end(), "SmartHome") != listResp["schemas"].end());

  // --- 3️⃣ getSchema (new test) ---
  json getSchemaReq = {{"command", "getSchema"}, {"schemaName", "SmartHome"}};
  auto schemaDetails = json::parse(router.handleRequest(getSchemaReq.dump()));

  // ✅ Top-level checks
  REQUIRE(schemaDetails["status"] == "ok");
  REQUIRE(schemaDetails.contains("schema"));
  REQUIRE(schemaDetails["schema"].is_object());

  // ✅ Now drill into schema object
  auto schemaObj = schemaDetails["schema"];
  REQUIRE(schemaObj["name"] == "SmartHome");
  REQUIRE(schemaObj["fields"].is_object());
  REQUIRE(schemaObj["children"].is_object());

  // --- 4️⃣ Load data ---
  json dataReq = {
      {"command", "loadData"},
      {"data", {{"homes.yaml", R"(
home1:
  _schema: SmartHome
  name: Villa Aurora
)"},
                {"devices.yaml", R"(
device1:
  _schema: Device
  _parentid: home1
  name: Thermostat
  active: true
)"}}}};

  auto dataResp = json::parse(router.handleRequest(dataReq.dump()));
  REQUIRE(dataResp["status"] == "ok");

  // --- 5️⃣ Query entity ---
  json queryReq = {{"command", "queryEntity"}, {"id", "device1"}};
  auto queryResp = json::parse(router.handleRequest(queryReq.dump()));
  REQUIRE(queryResp["status"] == "ok");
  REQUIRE(queryResp["entity"]["name"] == "Thermostat");

  // --- 6️⃣ setField ---
  json setReq = {
      {"command", "setField"},
      {"entityId", "device1"},
      {"fieldName", "name"},
      {"value", "ThermoX"}};
  auto setResp = json::parse(router.handleRequest(setReq.dump()));
  REQUIRE(setResp["status"] == "ok");

  // Confirm field was updated
  auto updated = json::parse(router.handleRequest(queryReq.dump()));
  REQUIRE(updated["entity"]["name"] == "ThermoX");

  // --- 7️⃣ validateEntity ---
  json validateReq = {{"command", "validateEntity"}, {"entityId", "device1"}};
  auto validResp = json::parse(router.handleRequest(validateReq.dump()));
  REQUIRE(validResp["status"] == "ok");

  // --- 8️⃣ getTree ---
  json treeReq = {{"command", "getTree"}};
  auto treeResp = json::parse(router.handleRequest(treeReq.dump()));
  REQUIRE(treeResp["status"] == "ok");
  REQUIRE(treeResp["tree"].is_array());

  // ✅ Find home1 node
  auto homeNode = std::find_if(treeResp["tree"].begin(), treeResp["tree"].end(),
                               [](const json &n)
                               { return n["id"] == "home1"; });
  REQUIRE(homeNode != treeResp["tree"].end());

  // --- 9️⃣ getRoot (NEW) ---
  json rootReq = {{"command", "getRoot"}};
  auto rootResp = json::parse(router.handleRequest(rootReq.dump()));
  REQUIRE(rootResp["status"] == "ok");
  REQUIRE(rootResp["root"].is_array());
  REQUIRE(rootResp["root"][0]["id"] == "home1");

  // --- 🔟 getChildren (NEW) ---
  json childrenReq = {{"command", "getChildren"}, {"parentId", "home1"}};
  auto childrenResp = json::parse(router.handleRequest(childrenReq.dump()));
  REQUIRE(childrenResp["status"] == "ok");
  REQUIRE(childrenResp["children"].is_array());

  // ✅ Look for device1 among the children
  auto childIt = std::find_if(childrenResp["children"].begin(),
                              childrenResp["children"].end(),
                              [](const json &child)
                              {
                                return child["id"] == "device1";
                              });
  REQUIRE(childIt != childrenResp["children"].end());
}

TEST_CASE("ToorCraftRouter handles errors gracefully")
{
  auto &router = ToorCraftRouter::instance();

  // --- 🚨 Unknown command ---
  json badCmd = {{"command", "noSuchCommand"}};
  auto badResp = json::parse(router.handleRequest(badCmd.dump()));
  REQUIRE(badResp["status"] == "error");
  REQUIRE(badResp["message"].is_string());

  // --- 🚨 Missing command field ---
  json missingCmd = {{"schemas", {{"file.yaml", "content"}}}};
  auto missingResp = json::parse(router.handleRequest(missingCmd.dump()));
  REQUIRE(missingResp["status"] == "error");

  // --- 🚨 setField on missing entity ---
  json schemas = {
      {"command", "loadSchemas"},
      {"schemas", {{"device.yaml", R"(
entity_name: Device
fields:
  name:
    type: string
)"}}}};
  REQUIRE(json::parse(router.handleRequest(schemas.dump()))["status"] == "ok");

  json setReq = {
      {"command", "setField"},
      {"entityId", "ghost"},
      {"fieldName", "name"},
      {"value", "fail"}};
  auto setResp = json::parse(router.handleRequest(setReq.dump()));
  REQUIRE(setResp["status"] == "error");
  REQUIRE(setResp["message"].is_string());
}
