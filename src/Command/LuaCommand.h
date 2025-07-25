#pragma once
#include "Command.h"
#include <string>
#include <unordered_map>

struct LuaCommandConfig : public CommandConfig {
    std::string scriptPath;

    // Arbitrary key-value params accessible by the Lua command
    std::unordered_map<std::string, std::string> params;

    virtual ~LuaCommandConfig() = default;
};

class LuaCommand : public Command {
public:
    explicit LuaCommand(const LuaCommandConfig& config);
    virtual ~LuaCommand() = default;

    // Now includes error output string parameter
    bool execute(const Entity& entity,
             const std::unordered_map<std::string, std::string>& overrideParams,
             std::string& error) override;

private:
    LuaCommandConfig config_;
};