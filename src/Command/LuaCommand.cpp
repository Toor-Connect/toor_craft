#include "LuaCommand.h"
#include "LuaManager.h"

LuaCommand::LuaCommand(const LuaCommandConfig& config)
    : Command(config), config_(config)
{
}

bool LuaCommand::execute(const Entity& entity,
                         const std::unordered_map<std::string, std::string>& overrideParams,
                         std::string& error)
{
    // Merge overrideParams into config_.params (override has priority)
    std::unordered_map<std::string, std::string> finalParams = config_.params;
    for (const auto& [key, value] : overrideParams) {
        finalParams[key] = value;
    }

    std::string internalError;
    bool result = LuaManager::instance().runScript(config_.scriptPath, const_cast<Entity&>(entity), finalParams, internalError);

    if (!result) {
        error = std::move(internalError);
    }

    return result;
}
