#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "ToorCraftRouter.h"

namespace fs = std::filesystem;

static void loadYamlFiles(const fs::path &dir, std::unordered_map<std::string, std::string> &map)
{
    for (auto &p : fs::recursive_directory_iterator(dir))
    {
        if (p.is_regular_file() && (p.path().extension() == ".yaml" || p.path().extension() == ".yml"))
        {
            std::ifstream in(p.path());
            if (!in)
                continue;

            std::ostringstream buffer;
            buffer << in.rdbuf();
            map[p.path().filename().string()] = buffer.str();
        }
    }
}

int main(int argc, char *argv[])
{
    fs::path schemaDir;
    fs::path dataDir;

    // --- Parse CLI arguments ---
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--schemas" && i + 1 < argc)
        {
            schemaDir = argv[++i];
        }
        else if (arg == "--data" && i + 1 < argc)
        {
            dataDir = argv[++i];
        }
    }

    if (schemaDir.empty() || dataDir.empty())
    {
        std::cerr << R"({"status": "error", "message": "Usage: toorcraft-cli --schemas <path> --data <path>"})" << std::endl;
        return 1;
    }

    // --- Load YAMLs ---
    std::unordered_map<std::string, std::string> schemas;
    std::unordered_map<std::string, std::string> data;

    loadYamlFiles(schemaDir, schemas);
    loadYamlFiles(dataDir, data);

    if (schemas.empty())
    {
        std::cerr << R"({"status": "error", "message": "No schema files found"})" << std::endl;
        return 1;
    }
    if (data.empty())
    {
        std::cerr << R"({"status": "error", "message": "No data files found"})" << std::endl;
        return 1;
    }

    // --- Use Router for everything ---
    auto &router = ToorCraftRouter::instance();

    // ✅ Load schemas
    nlohmann::json loadSchemasReq;
    loadSchemasReq["command"] = "loadSchemas";
    loadSchemasReq["payload"] = schemas;

    std::string schemaResp = router.handleRequest(loadSchemasReq.dump());
    if (schemaResp.find("\"status\":\"error\"") != std::string::npos)
    {
        std::cerr << schemaResp << std::endl;
        return 1;
    }

    // ✅ Load data
    nlohmann::json loadDataReq;
    loadDataReq["command"] = "loadData";
    loadDataReq["payload"] = data;

    std::string dataResp = router.handleRequest(loadDataReq.dump());
    if (dataResp.find("\"status\":\"error\"") != std::string::npos)
    {
        std::cerr << dataResp << std::endl;
        return 1;
    }

    // ✅ Read stdin for runtime commands
    std::ostringstream buffer;
    std::string line;
    while (std::getline(std::cin, line))
    {
        buffer << line << "\n";
    }

    std::string input = buffer.str();
    if (input.empty())
    {
        std::cerr << R"({"status": "error", "message": "No JSON input provided"})" << std::endl;
        return 1;
    }

    // ✅ Route the request
    std::string response = router.handleRequest(input);
    std::cout << response << std::endl;

    return 0;
}
