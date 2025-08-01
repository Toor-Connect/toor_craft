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
    bool interactive = false;

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
        else if (arg == "--interactive")
        {
            interactive = true;
        }
        else if (arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " --schemas <path> --data <path> [--interactive]\n";
            return 0;
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
    loadSchemasReq["schemas"] = schemas;

    auto schemaRespJson = nlohmann::json::parse(router.handleRequest(loadSchemasReq.dump()));
    if (schemaRespJson["status"] == "error")
    {
        std::cerr << schemaRespJson.dump(2) << std::endl;
        return 1;
    }

    // ✅ Load data
    nlohmann::json loadDataReq;
    loadDataReq["command"] = "loadData";
    loadDataReq["data"] = data;

    auto dataRespJson = nlohmann::json::parse(router.handleRequest(loadDataReq.dump()));
    if (dataRespJson["status"] == "error")
    {
        std::cerr << dataRespJson.dump(2) << std::endl;
        return 1;
    }

    if (interactive)
    {
        std::cout << "✅ ToorCraft CLI ready. Type JSON commands and press Enter.\n";
        std::cout << "ℹ️  Type 'help' for example commands, 'exit' to quit.\n";

        std::string line;
        while (true)
        {
            std::cout << "> ";
            if (!std::getline(std::cin, line))
                break;
            if (line == "exit" || line == "quit")
                break;
            if (line == "help")
            {
                std::cout << R"({
  "examples": [
    {"command": "getTree"},
    {"command": "getSchemaList"},
    {"command": "queryEntity", "id": "device1"},
    {"command": "setField", "id": "device1", "field": "name", "value": "NewName"},
    {"command": "validateEntity", "id": "device1"}
  ]
})" << std::endl;
                continue;
            }
            if (line.empty())
                continue;

            try
            {
                std::string response = router.handleRequest(line);

                // Pretty print response
                nlohmann::json parsed = nlohmann::json::parse(response);
                std::cout << parsed.dump(2) << "\n";
            }
            catch (const std::exception &ex)
            {
                std::cerr << "\033[31m"
                          << R"({"status":"error","message":")" << ex.what() << "\"}"
                          << "\033[0m" << std::endl;
            }
        }
    }
    else
    {
        // ✅ Non-interactive: read stdin all at once
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

        std::string response = router.handleRequest(input);
        nlohmann::json parsed = nlohmann::json::parse(response);
        std::cout << parsed.dump(2) << std::endl;
    }

    return 0;
}
