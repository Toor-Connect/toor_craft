#include <catch2/catch_test_macros.hpp>
#include "ToorCraftEngine.h"
#include "Entity.h"

TEST_CASE("ToorCraftEngine handles invalid input by throwing exceptions")
{
    ToorCraftEngine &engine = ToorCraftEngine::instance();

    SECTION("Throws on schema missing profile/entity name")
    {
        std::unordered_map<std::string, std::string> badSchemas;
        badSchemas["broken.yaml"] = R"(
fields:
  name:
    type: string
)";

        REQUIRE_THROWS_AS(engine.loadSchemas(badSchemas), std::runtime_error);
    }

    SECTION("Throws on duplicate schema name")
    {
        std::unordered_map<std::string, std::string> schemas;
        schemas["schema1.yaml"] = R"(
entity_name: Device
)";
        schemas["schema2.yaml"] = R"(
entity_name: Device
)";

        REQUIRE_THROWS_AS(engine.loadSchemas(schemas), std::runtime_error);
    }

    SECTION("Throws when data references unknown schema")
    {
        std::unordered_map<std::string, std::string> schemas;
        schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name:
    type: string
)";
        REQUIRE_NOTHROW(engine.loadSchemas(schemas));

        std::unordered_map<std::string, std::string> data;
        data["bad.yaml"] = R"(
ghost1:
  _schema: GhostSchema
  name: Boo
)";

        REQUIRE_THROWS_AS(engine.loadData(data), std::runtime_error);
    }

    SECTION("Throws when entity defines field not in schema")
    {
        std::unordered_map<std::string, std::string> schemas;
        schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name:
    type: string
)";
        REQUIRE_NOTHROW(engine.loadSchemas(schemas));

        std::unordered_map<std::string, std::string> data;
        data["badField.yaml"] = R"(
device1:
  _schema: Device
  name: OK
  rogue_field: ShouldNotExist
)";

        REQUIRE_THROWS_AS(engine.loadData(data), std::runtime_error);
    }

    SECTION("Throws when required field is missing")
    {
        std::unordered_map<std::string, std::string> schemas;
        schemas["device.yaml"] = R"(
entity_name: Device
fields:
  name:
    type: string
    required: true
)";
        REQUIRE_NOTHROW(engine.loadSchemas(schemas));

        std::unordered_map<std::string, std::string> data;
        data["missing.yaml"] = R"(
deviceX:
  _schema: Device
)";

        REQUIRE_NOTHROW(engine.loadData(data));

        // ✅ validateEntity now throws on missing required fields
        REQUIRE_THROWS_AS(engine.validateEntity("deviceX"), std::runtime_error);
    }

    SECTION("Returns nullptr when querying a non-existent entity")
    {
        Entity *e = engine.queryEntity("nonexistent");
        REQUIRE(e == nullptr);
    }

    SECTION("Throws when setting field on missing entity")
    {
        REQUIRE_THROWS_AS(engine.setField("ghost", "name", "test"), std::runtime_error);
    }
}
