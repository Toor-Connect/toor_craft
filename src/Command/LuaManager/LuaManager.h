#pragma once
#include <string>
#include <unordered_map>
#include <Entity.h>

class LuaManager
{
public:
    static LuaManager &instance();
    void runScript(const std::string &scriptPath_,
                   const Entity &entity,
                   const std::unordered_map<std::string, std::string> &params);

private:
    class LuaManagerImpl;
    LuaManagerImpl *impl_;

    LuaManager();
    ~LuaManager();

    LuaManager(const LuaManager &) = delete;
    LuaManager &operator=(const LuaManager &) = delete;
};
