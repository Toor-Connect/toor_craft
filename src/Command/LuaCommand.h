#pragma once
#include "Command.h"
#include <string>
#include <unordered_map>

struct LuaCommandConfig : public CommandConfig
{
    std::string scriptPath;
    std::unordered_map<std::string, std::string> params;
};

class LuaCommand : public Command
{
public:
    explicit LuaCommand(LuaCommandConfig config);
    virtual ~LuaCommand() = default;
    void execute(const Entity &entity) const override;

private:
    std::string scriptPath_;
    std::unordered_map<std::string, std::string> params_;
};