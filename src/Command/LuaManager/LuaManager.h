#pragma once
#include <string>
#include <unordered_map>
#include <Entity.h>

class LuaManager
{
public:
    static LuaManager &instance();
    void setBaseDirectory(const std::string &baseDir);
    void runScript(const std::string &scriptName,
                   const std::string &scriptContent,
                   const Entity &entity,
                   const std::unordered_map<std::string, std::string> &params);

private:
    // Implementation class (nested private)
    std::string baseDirectory_;
    class LuaManagerImpl;
    LuaManagerImpl *impl_;

    LuaManager();
    ~LuaManager();

    LuaManager(const LuaManager &) = delete;
    LuaManager &operator=(const LuaManager &) = delete;
};
