#pragma once
#include "Command.h"
#include <string>
#include <unordered_map>

struct LuaCommandConfig : public CommandConfig
{
    std::string scriptName;
    std::string scriptContent;
    std::unordered_map<std::string, std::string> params;
};

class LuaCommand : public Command
{
public:
    explicit LuaCommand(LuaCommandConfig config);
    virtual ~LuaCommand() = default;
    void execute(const Entity &entity) const override;

private:
    std::string scriptName_;
    std::string scriptContent_;
    std::unordered_map<std::string, std::string> params_;
};