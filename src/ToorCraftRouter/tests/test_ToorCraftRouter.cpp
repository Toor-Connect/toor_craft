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
  json getSchemaReq = {{"command", "getSchema"}, {"schema", "SmartHome"}};
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
      {"id", "device1"},
      {"field", "name"},
      {"value", "ThermoX"}};
  auto setResp = json::parse(router.handleRequest(setReq.dump()));
  REQUIRE(setResp["status"] == "ok");

  // Confirm field was updated
  auto updated = json::parse(router.handleRequest(queryReq.dump()));
  REQUIRE(updated["entity"]["name"] == "ThermoX");

  // --- 7️⃣ validateEntity ---
  json validateReq = {{"command", "validateEntity"}, {"id", "device1"}};
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

  // --- 1️⃣ Load schema ---
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

  // --- 2️⃣ createEntity (root) ---
  json homeCreate = {
      {"command", "createEntity"},
      {"schema", "SmartHome"},
      {"id", "homeZ"},
      {"payload", {{"name", "Villa Nova"}}}};
  auto homeResp = json::parse(router.handleRequest(homeCreate.dump()));
  REQUIRE(homeResp["status"] == "ok");
  REQUIRE(homeResp["created"]["id"] == "homeZ");
  REQUIRE(homeResp["created"]["schema"] == "SmartHome");
  REQUIRE(homeResp["created"]["parentId"].is_null());

  // --- 3️⃣ createEntity (child of homeZ) ---
  json deviceCreate = {
      {"command", "createEntity"},
      {"schema", "Device"},
      {"id", "deviceZ"},
      {"parentId", "homeZ"},
      {"payload", {{"name", "Thermo Deluxe"}}}};
  auto deviceResp = json::parse(router.handleRequest(deviceCreate.dump()));
  REQUIRE(deviceResp["status"] == "ok");
  REQUIRE(deviceResp["created"]["id"] == "deviceZ");
  REQUIRE(deviceResp["created"]["schema"] == "Device");
  REQUIRE(deviceResp["created"]["parentId"] == "homeZ");

  // --- 4️⃣ getParent for deviceZ ---
  json parentReq = {
      {"command", "getParent"},
      {"id", "deviceZ"}};
  auto parentResp = json::parse(router.handleRequest(parentReq.dump()));
  REQUIRE(parentResp["status"] == "ok");
  REQUIRE(parentResp["parent"]["id"] == "homeZ");
  REQUIRE(parentResp["parent"]["schema"] == "SmartHome");

  // ✅ Also check that getParent on a root entity returns null parent
  json parentRootReq = {
      {"command", "getParent"},
      {"id", "homeZ"}};
  auto parentRootResp = json::parse(router.handleRequest(parentRootReq.dump()));
  REQUIRE(parentRootResp["status"] == "ok");
  REQUIRE(parentRootResp["parent"].is_null());
}

TEST_CASE("ToorCraftRouter handles deep cascade deletion and reference cleanup")
{
  auto &router = ToorCraftRouter::instance();

  // --- 1️⃣ Load schema with 3 levels and references ---
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

  // --- 2️⃣ Create a home (root) ---
  json homeCreate = {
      {"command", "createEntity"},
      {"schema", "SmartHome"},
      {"id", "homeCascade"},
      {"payload", {{"name", "Casa Grande"}}}};
  auto homeResp = json::parse(router.handleRequest(homeCreate.dump()));
  REQUIRE(homeResp["status"] == "ok");

  // --- 3️⃣ Create two devices under the home ---
  json devACreate = {
      {"command", "createEntity"},
      {"schema", "Device"},
      {"id", "deviceA"},
      {"parentId", "homeCascade"},
      {"payload", {{"name", "Device Alpha"}}}};
  REQUIRE(json::parse(router.handleRequest(devACreate.dump()))["status"] == "ok");

  json devBCreate = {
      {"command", "createEntity"},
      {"schema", "Device"},
      {"id", "deviceB"},
      {"parentId", "homeCascade"},
      {"payload", {{"name", "Device Beta"}}}};
  REQUIRE(json::parse(router.handleRequest(devBCreate.dump()))["status"] == "ok");

  // --- 4️⃣ Create sensors under deviceA ---
  json sensorA1Create = {
      {"command", "createEntity"},
      {"schema", "Sensor"},
      {"id", "sensorA1"},
      {"parentId", "deviceA"},
      {"payload", {{"name", "Temp Sensor"}}}};
  REQUIRE(json::parse(router.handleRequest(sensorA1Create.dump()))["status"] == "ok");

  json sensorA2Create = {
      {"command", "createEntity"},
      {"schema", "Sensor"},
      {"id", "sensorA2"},
      {"parentId", "deviceA"},
      {"payload", {{"name", "Humidity Sensor"}}}};
  REQUIRE(json::parse(router.handleRequest(sensorA2Create.dump()))["status"] == "ok");

  // --- 5️⃣ Cross-reference deviceA & deviceB ---
  json setSiblingA = {
      {"command", "setField"},
      {"id", "deviceA"},
      {"field", "sibling"},
      {"value", "deviceB"}};
  REQUIRE(json::parse(router.handleRequest(setSiblingA.dump()))["status"] == "ok");

  json setSiblingB = {
      {"command", "setField"},
      {"id", "deviceB"},
      {"field", "sibling"},
      {"value", "deviceA"}};
  REQUIRE(json::parse(router.handleRequest(setSiblingB.dump()))["status"] == "ok");

  // 🔍 Confirm references before deletion
  auto devA = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceA"})"));
  auto devB = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceB"})"));
  REQUIRE(devA["entity"]["sibling"] == "deviceB");
  REQUIRE(devB["entity"]["sibling"] == "deviceA");

  // 🔍 Confirm sensors exist before deletion
  auto sensorCheck = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"sensorA1"})"));
  REQUIRE(sensorCheck["status"] == "ok");
  REQUIRE(sensorCheck["entity"]["name"] == "Temp Sensor");

  // --- 6️⃣ Delete deviceA (should also delete its sensors) ---
  json deleteDevA = {
      {"command", "deleteEntity"},
      {"id", "deviceA"}};
  auto deleteRespA = json::parse(router.handleRequest(deleteDevA.dump()));
  REQUIRE(deleteRespA["status"] == "ok");

  // 🔍 Query deviceA should return not_found
  auto devAQuery = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceA"})"));
  REQUIRE(devAQuery["status"] == "not_found");

  // 🔍 Sensors under deviceA should also be deleted
  auto sensorAfter1 = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"sensorA1"})"));
  auto sensorAfter2 = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"sensorA2"})"));
  REQUIRE(sensorAfter1["status"] == "not_found");
  REQUIRE(sensorAfter2["status"] == "not_found");

  // 🔍 DeviceB should now have sibling cleared (null or empty)
  auto devBAfter = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceB"})"));
  REQUIRE(devBAfter["status"] == "ok");
  REQUIRE((!devBAfter["entity"].contains("sibling") ||
           devBAfter["entity"]["sibling"].is_null() ||
           devBAfter["entity"]["sibling"] == ""));

  // --- 7️⃣ Delete home (should remove deviceB too) ---
  json deleteHome = {
      {"command", "deleteEntity"},
      {"id", "homeCascade"}};
  auto deleteHomeResp = json::parse(router.handleRequest(deleteHome.dump()));
  REQUIRE(deleteHomeResp["status"] == "ok");

  // 🔍 DeviceB should now be gone
  auto devBQuery = json::parse(router.handleRequest(R"({"command":"queryEntity","id":"deviceB"})"));
  REQUIRE(devBQuery["status"] == "not_found");

  // 🔍 Tree should now be empty
  auto treeAfter = json::parse(router.handleRequest(R"({"command":"getTree"})"));
  REQUIRE(treeAfter["status"] == "ok");
  REQUIRE(treeAfter["tree"].empty());
}
