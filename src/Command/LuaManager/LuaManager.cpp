#include "LuaManager.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
#include <unordered_map>

class LuaManager::LuaManagerImpl {
public:
    lua_State* L;

    LuaManagerImpl() {
        L = luaL_newstate();
        if (L) {
            luaL_openlibs(L);
        }
    }

    ~LuaManagerImpl() {
        if (L) {
            lua_close(L);
            L = nullptr;
        }
    }

    void pushParamsTable(const std::unordered_map<std::string, std::string>& params) {
        lua_newtable(L);
        for (const auto& [key, value] : params) {
            lua_pushstring(L, value.c_str());
            lua_setfield(L, -2, key.c_str());
        }
    }

    void pushEntity(const Entity& entity) {
        lua_pushstring(L, entity.getId().c_str());
    }

    bool runScript(const std::string& scriptPath,
                   const Entity& entity,
                   const std::unordered_map<std::string, std::string>& params,
                   std::string& error) 
    {
        if (!L) {
            error = "Lua state not initialized";
            return false;
        }

        int loadStatus = luaL_loadfile(L, scriptPath.c_str());
        if (loadStatus != LUA_OK) {
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }

        pushEntity(entity);
        pushParamsTable(params);

        int callStatus = lua_pcall(L, 2, 0, 0);
        if (callStatus != LUA_OK) {
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }

        return true;
    }
};

LuaManager& LuaManager::instance() {
    static LuaManager instance;
    return instance;
}

LuaManager::LuaManager()
    : impl_(new LuaManagerImpl()) {}

LuaManager::~LuaManager() {
    delete impl_;
}

bool LuaManager::runScript(const std::string& scriptPath, Entity& entity,
                           const std::unordered_map<std::string, std::string>& params,
                           std::string& error) 
{
    return impl_->runScript(scriptPath, entity, params, error);
}
