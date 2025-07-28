#include "LuaCommand.h"
#include "LuaManager.h"

LuaCommand::LuaCommand(LuaCommandConfig config)
    : Command(std::move(config)),
      scriptPath_(std::move(config.scriptPath)),
      params_(std::move(config.params))
{
}

void LuaCommand::execute(const Entity &entity) const
{
    LuaManager::instance().runScript(scriptPath_, entity, params_);
}