#include <catch2/catch_test_macros.hpp>
#include "SchemaManager.h"
#include "EntitySchema.h"

TEST_CASE("SchemaManager handles a simple profile and entity")
{
    std::unordered_map<std::string, std::string> schemas;

    schemas["profile.yaml"] = R"(
profile_name: SmartHome
children:
  Device:
    entity_name: Device
)";

    schemas["device.yaml"] = R"(
entity_name: Device
)";

    SchemaManager &mgr = SchemaManager::instance();
    mgr.parseSchemaBundle(schemas);

    REQUIRE(mgr.profileExists("SmartHome"));
    REQUIRE(mgr.getEntity("Device") != nullptr);
    REQUIRE(mgr.getProfile("SmartHome")->getChildrenNames().size() == 1);
}

TEST_CASE("SchemaManager handles deep hierarchies and multiple profiles")
{
    std::unordered_map<std::string, std::string> schemas;

    // Profile 1
    schemas["factory.yaml"] = R"(
profile_name: FactoryProfile
fields:
  - name: id
    type: string
children:
  Line:
    entity_name: Line
  Worker:
    entity_name: Worker
)";

    // Profile 2
    schemas["office.yaml"] = R"(
profile_name: OfficeProfile
children:
  Desk:
    entity_name: Desk
)";

    // Entities for FactoryProfile
    schemas["line.yaml"] = R"(
entity_name: Line
children:
  Machine:
    entity_name: Machine
)";

    schemas["worker.yaml"] = R"(
entity_name: Worker
fields:
  - name: name
    type: string
)";

    schemas["machine.yaml"] = R"(
entity_name: Machine
children:
  Sensor:
    entity_name: Sensor
)";

    schemas["sensor.yaml"] = R"(
entity_name: Sensor
fields:
  - name: serial
    type: string
)";

    // Entities for OfficeProfile
    schemas["desk.yaml"] = R"(
entity_name: Desk
children:
  PC:
    entity_name: PC
)";

    schemas["pc.yaml"] = R"(
entity_name: PC
)";

    SchemaManager &mgr = SchemaManager::instance();
    mgr.parseSchemaBundle(schemas);

    SECTION("Profiles are correctly registered")
    {
        REQUIRE(mgr.profileExists("FactoryProfile"));
        REQUIRE(mgr.profileExists("OfficeProfile"));

        REQUIRE(mgr.getProfileNames().size() == 2);
    }

    SECTION("All entities exist in the global lookup")
    {
        auto names = mgr.getEntityNames();
        REQUIRE(names.size() == schemas.size()); // every YAML file produced one entity
        REQUIRE(mgr.getEntity("Machine") != nullptr);
        REQUIRE(mgr.getEntity("PC") != nullptr);
    }

    SECTION("Hierarchy linking is correct for FactoryProfile")
    {
        auto factory = mgr.getProfile("FactoryProfile");

        // Check children of FactoryProfile
        auto topChildren = factory->getChildrenNames();
        REQUIRE(topChildren.size() == 2);
        REQUIRE(std::find(topChildren.begin(), topChildren.end(), "Line") != topChildren.end());
        REQUIRE(std::find(topChildren.begin(), topChildren.end(), "Worker") != topChildren.end());

        // Check nested hierarchy: FactoryProfile -> Line -> Machine -> Sensor
        auto line = factory->getChildSchema("Line");
        REQUIRE(line != nullptr);
        auto lineChildren = line->getChildrenNames();
        REQUIRE(lineChildren.size() == 1);
        REQUIRE(lineChildren[0] == "Machine");

        auto machine = line->getChildSchema("Machine");
        REQUIRE(machine != nullptr);
        auto machineChildren = machine->getChildrenNames();
        REQUIRE(machineChildren.size() == 1);
        REQUIRE(machineChildren[0] == "Sensor");

        auto sensor = machine->getChildSchema("Sensor");
        REQUIRE(sensor != nullptr);
        REQUIRE(sensor->getFields().size() == 1);
    }

    SECTION("Hierarchy linking is correct for OfficeProfile")
    {
        auto office = mgr.getProfile("OfficeProfile");
        auto officeChildren = office->getChildrenNames();
        REQUIRE(officeChildren.size() == 1);
        REQUIRE(officeChildren[0] == "Desk");

        auto desk = office->getChildSchema("Desk");
        REQUIRE(desk != nullptr);
        REQUIRE(desk->getChildrenNames().size() == 1);
        REQUIRE(desk->getChildSchema("PC") != nullptr);
    }
}

TEST_CASE("SchemaManager throws when schema is invalid")
{
    std::unordered_map<std::string, std::string> badSchemas;

    // Missing profile_name and entity_name
    badSchemas["broken.yaml"] = R"(
name: Invalid
)";

    SchemaManager &mgr = SchemaManager::instance();

    REQUIRE_THROWS_AS(mgr.parseSchemaBundle(badSchemas), std::runtime_error);

    // Dangling child reference
    std::unordered_map<std::string, std::string> danglingSchemas;
    danglingSchemas["profile.yaml"] = R"(
profile_name: BrokenProfile
children:
  MissingChild:
    entity_name: MissingChild
)";
    // No file for MissingChild

    REQUIRE_THROWS_AS(mgr.parseSchemaBundle(danglingSchemas), std::runtime_error);
}
