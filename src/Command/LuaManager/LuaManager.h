#pragma once
#include <string>
#include <unordered_map>
#include <Entity.h>

class LuaManager {
public:
    static LuaManager& instance();
    bool runScript(const std::string& scriptPath,
                   Entity& entity,
                   const std::unordered_map<std::string, std::string>& params,
                   std::string& error);

private:
    // Implementation class (nested private)
    class LuaManagerImpl;
    LuaManagerImpl* impl_;

    LuaManager();
    ~LuaManager();

    LuaManager(const LuaManager&) = delete;
    LuaManager& operator=(const LuaManager&) = delete;
};
