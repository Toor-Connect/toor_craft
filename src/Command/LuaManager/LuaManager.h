#pragma once
#include <string>
#include <unordered_map>
#include <Entity.h>
#include <filesystem>

class LuaManager
{
public:
    static LuaManager &instance();
    void setBasePath(const std::filesystem::path &basePath);
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
