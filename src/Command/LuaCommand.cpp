#include "LuaCommand.h"
#include "LuaManager.h"

LuaCommand::LuaCommand(LuaCommandConfig config)
    : Command(std::move(config)),
      scriptName_(std::move(config.scriptName)),
      scriptContent_(std::move(config.scriptContent)),
      params_(std::move(config.params))
{
}

void LuaCommand::execute(const Entity &entity) const
{
    LuaManager::instance().runScript(scriptName_, scriptContent_, entity, params_);
}