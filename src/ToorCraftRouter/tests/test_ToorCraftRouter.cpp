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

  // --- 3️⃣ getSchema ---
  json getSchemaReq = {{"command", "getSchema"}, {"schema", "SmartHome"}};
  auto schemaDetails = json::parse(router.handleRequest(getSchemaReq.dump()));

  REQUIRE(schemaDetails["status"] == "ok");
  REQUIRE(schemaDetails.contains("schema"));
  REQUIRE(schemaDetails["schema"].is_object());

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
  REQUIRE(queryResp["entity"]["state"] == "Unchanged"); // ✅ Loaded data → state = Unchanged

  // --- 6️⃣ setField ---
  json setReq = {
      {"command", "setField"},
      {"id", "device1"},
      {"field", "name"},
      {"value", "ThermoX"}};
  auto setResp = json::parse(router.handleRequest(setReq.dump()));
  REQUIRE(setResp["status"] == "ok");

  // ✅ Query again to check Modified state
  auto updated = json::parse(router.handleRequest(queryReq.dump()));
  REQUIRE(updated["entity"]["name"] == "ThermoX");
  REQUIRE(updated["entity"]["state"] == "Modified");

  // --- 7️⃣ validateEntity ---
  json validateReq = {{"command", "validateEntity"}, {"id", "device1"}};
  auto validResp = json::parse(router.handleRequest(validateReq.dump()));
  REQUIRE(validResp["status"] == "ok");

  // --- 8️⃣ getTree ---
  json treeReq = {{"command", "getTree"}};
  auto treeResp = json::parse(router.handleRequest(treeReq.dump()));
  REQUIRE(treeResp["status"] == "ok");
  REQUIRE(treeResp["tree"].is_array());

  auto homeNode = std::find_if(treeResp["tree"].begin(), treeResp["tree"].end(),
                               [](const json &n)
                               { return n["id"] == "home1"; });
  REQUIRE(homeNode != treeResp["tree"].end());
}

TEST_CASE("ToorCraftRouter handles errors gracefully")
{
  auto &router = ToorCraftRouter::instance();

  // 🚨 Unknown command
  json badCmd = {{"command", "noSuchCommand"}};
  auto badResp = json::parse(router.handleRequest(badCmd.dump()));
  REQUIRE(badResp["status"] == "error");
  REQUIRE(badResp["message"].is_string());

  // 🚨 Missing command field
  json missingCmd = {{"schemas", {{"file.yaml", "content"}}}};
  auto missingResp = json::parse(router.handleRequest(missingCmd.dump()));
  REQUIRE(missingResp["status"] == "error");

  // 🚨 setField on missing entity
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
      {"id", "ghost"},
      {"field", "name"},
      {"value", "fail"}};
  auto setResp = json::parse(router.handleRequest(setReq.dump()));
  REQUIRE(setResp["status"] == "error");
  REQUIRE(setResp["message"].is_string());
}

TEST_CASE("ToorCraftRouter supports createEntity and getParent")
{
  auto &router = ToorCraftRouter::instance();

  // 1️⃣ Load schema
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
)"},
                   {"device.yaml", R"(
entity_name: Device
fields:
  name:
    type: string
    required: true
)"}}}};
  REQUIRE(json::parse(router.handleRequest(schemaReq.dump()))["status"] == "ok");

  // 2️⃣ createEntity (root)
  router.handleRequest(R"({"command":"createEntity","schema":"SmartHome","id":"homeZ","payload":{"name":"Villa Nova"}})");
  auto homeQuery = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"homeZ"})"));
  REQUIRE(homeQuery["entity"]["state"] == "Added"); // ✅ Creation → Added

  // 3️⃣ createEntity (child)
  router.handleRequest(R"({"command":"createEntity","schema":"Device","id":"deviceZ","parentId":"homeZ","payload":{"name":"Thermo Deluxe"}})");
  auto deviceQuery = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceZ"})"));
  REQUIRE(deviceQuery["entity"]["state"] == "Added");

  // 4️⃣ getParent for deviceZ
  json parentReq = {{"command", "getParent"}, {"id", "deviceZ"}};
  auto parentResp = json::parse(router.handleRequest(parentReq.dump()));
  REQUIRE(parentResp["status"] == "ok");
  REQUIRE(parentResp["parent"]["id"] == "homeZ");
}

TEST_CASE("ToorCraftRouter handles deep cascade deletion and reference cleanup")
{
  auto &router = ToorCraftRouter::instance();

  // 1️⃣ Load schema
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
)"},
                   {"device.yaml", R"(
entity_name: Device
children:
  sensors:
    entity: Sensor
fields:
  name:
    type: string
  sibling:
    type: reference
    target: Device
)"},
                   {"sensor.yaml", R"(
entity_name: Sensor
fields:
  name:
    type: string
)"}}}};
  REQUIRE(json::parse(router.handleRequest(schemaReq.dump()))["status"] == "ok");

  // 2️⃣ Create hierarchy: home → devices → sensors
  router.handleRequest(R"({"command":"createEntity","schema":"SmartHome","id":"homeCascade","payload":{"name":"Casa Grande"}})");
  router.handleRequest(R"({"command":"createEntity","schema":"Device","id":"deviceA","parentId":"homeCascade","payload":{"name":"Alpha"}})");
  router.handleRequest(R"({"command":"createEntity","schema":"Device","id":"deviceB","parentId":"homeCascade","payload":{"name":"Beta"}})");
  router.handleRequest(R"({"command":"createEntity","schema":"Sensor","id":"sensorA1","parentId":"deviceA","payload":{"name":"Temp Sensor"}})");
  router.handleRequest(R"({"command":"createEntity","schema":"Sensor","id":"sensorA2","parentId":"deviceA","payload":{"name":"Humidity Sensor"}})");

  // 3️⃣ Reference deviceA <-> deviceB
  router.handleRequest(R"({"command":"setField","id":"deviceA","field":"sibling","value":"deviceB"})");
  router.handleRequest(R"({"command":"setField","id":"deviceB","field":"sibling","value":"deviceA"})");

  // 4️⃣ Delete deviceA
  json deleteDevA = {{"command", "deleteEntity"}, {"id", "deviceA"}};
  auto deleteRespA = json::parse(router.handleRequest(deleteDevA.dump()));
  REQUIRE(deleteRespA["status"] == "ok");

  // ✅ DeviceA marked Deleted
  auto devAQuery = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceA"})"));
  REQUIRE(devAQuery["entity"]["state"] == "Deleted");

  // ✅ Its sensors marked Deleted
  auto sensorAfter1 = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"sensorA1"})"));
  REQUIRE(sensorAfter1["entity"]["state"] == "Deleted");

  // 5️⃣ Delete home (marks deviceB Deleted too)
  json deleteHome = {{"command", "deleteEntity"}, {"id", "homeCascade"}};
  auto deleteHomeResp = json::parse(router.handleRequest(deleteHome.dump()));
  REQUIRE(deleteHomeResp["status"] == "ok");

  auto devBQuery = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceB"})"));
  REQUIRE(devBQuery["entity"]["state"] == "Deleted");
}
