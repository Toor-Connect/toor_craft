#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "ToorCraftRouter.h"

using json = nlohmann::json;

TEST_CASE("ToorCraftRouter handles full lifecycle commands")
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
  tags:
    type: array
    element:
      type: string
)"},
                   {"device.yaml", R"(
entity_name: Device
fields:
  name:
    type: string
    required: true
  active:
    type: boolean
)"}}}};

  auto schemaResp = json::parse(router.handleRequest(schemaReq.dump()));
  REQUIRE(schemaResp["status"] == "ok");

  // --- 2️⃣ Verify schema list ---
  json listReq = {{"command", "getSchemaList"}};
  auto listResp = json::parse(router.handleRequest(listReq.dump()));
  REQUIRE(listResp["status"] == "ok");
  REQUIRE(listResp["schemas"].is_array());
  REQUIRE(std::find(listResp["schemas"].begin(), listResp["schemas"].end(), "SmartHome") != listResp["schemas"].end());
  REQUIRE(std::find(listResp["schemas"].begin(), listResp["schemas"].end(), "Device") != listResp["schemas"].end());

  // --- 3️⃣ Load data ---
  json dataReq = {
      {"command", "loadData"},
      {"data", {{"homes.yaml", R"(
home1:
  _schema: SmartHome
  name: Villa Aurora
  tags:
    - modern
    - solar
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

  // --- 4️⃣ Query entity ---
  json queryReq = {{"command", "queryEntity"}, {"id", "device1"}};
  auto queryResp = json::parse(router.handleRequest(queryReq.dump()));
  REQUIRE(queryResp["status"] == "ok");
  REQUIRE(queryResp["entity"]["name"] == "Thermostat");

  // --- 5️⃣ Update field ---
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

  // --- 6️⃣ Validate entity ---
  json validateReq = {{"command", "validateEntity"}, {"entityId", "device1"}};
  auto validResp = json::parse(router.handleRequest(validateReq.dump()));
  REQUIRE(validResp["status"] == "ok");

  // --- 7️⃣ Get tree ---
  json treeReq = {{"command", "getTree"}};
  auto treeResp = json::parse(router.handleRequest(treeReq.dump()));
  REQUIRE(treeResp["status"] == "ok");
  REQUIRE(treeResp["tree"].is_array());
  auto homeNode = std::find_if(treeResp["tree"].begin(), treeResp["tree"].end(),
                               [](const json &n)
                               { return n["id"] == "home1"; });
  REQUIRE(homeNode != treeResp["tree"].end());
  REQUIRE(std::find(homeNode->at("children").begin(),
                    homeNode->at("children").end(),
                    "device1") != homeNode->at("children").end());
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
