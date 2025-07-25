#include "LuaManager.h"
#include "EntityManager.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
#include <unordered_map>

class LuaManager::LuaManagerImpl
{
public:
    lua_State *L;

    LuaManagerImpl()
    {
        L = luaL_newstate();
        if (L)
        {
            luaL_openlibs(L);
            registerLuaFunctions();
        }
    }

    ~LuaManagerImpl()
    {
        if (L)
        {
            lua_close(L);
            L = nullptr;
        }
    }

    // Lua callable function: getField(entityId, fieldName)
    static int lua_getField(lua_State *L)
    {
        // Check and get args
        const char *entityId = luaL_checkstring(L, 1);
        const char *fieldName = luaL_checkstring(L, 2);

        // Fetch entity and field value
        Entity *entity = EntityManager::instance().getEntityById(entityId);
        if (!entity)
        {
            lua_pushnil(L);
            return 1;
        }

        FieldValue *fieldValue = entity->getFieldValue(fieldName);
        if (!fieldValue)
        {
            lua_pushnil(L);
            return 1;
        }

        // Push field value string representation to Lua stack
        lua_pushstring(L, fieldValue->toString().c_str());
        return 1; // Number of return values
    }

    void registerLuaFunctions()
    {
        // Register 'getField' function globally in Lua
        lua_register(L, "getField", lua_getField);
    }

    void pushParamsTable(const std::unordered_map<std::string, std::string> &params)
    {
        lua_newtable(L);
        for (const auto &[key, value] : params)
        {
            lua_pushstring(L, value.c_str());
            lua_setfield(L, -2, key.c_str());
        }
    }

    void pushEntity(const Entity &entity)
    {
        lua_pushstring(L, entity.getId().c_str());
    }

    bool runScript(const std::string &scriptPath,
                   const Entity &entity,
                   const std::unordered_map<std::string, std::string> &params,
                   std::string &error)
    {
        if (!L)
        {
            error = "Lua state not initialized";
            return false;
        }

        int loadStatus = luaL_loadfile(L, scriptPath.c_str());
        if (loadStatus != LUA_OK)
        {
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }

        pushEntity(entity);
        pushParamsTable(params);

        // We expect the Lua script to return 2 values: boolean success and string error message
        int callStatus = lua_pcall(L, 2, 2, 0);
        if (callStatus != LUA_OK)
        {
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }

        // Now read the two return values from the stack
        // Stack top is error message (string)
        if (!lua_isstring(L, -1))
        {
            error = "Lua script did not return error message string";
            lua_pop(L, 2); // Pop both returns
            return false;
        }
        error = lua_tostring(L, -1);
        lua_pop(L, 1);

        // Next is boolean success
        if (!lua_isboolean(L, -1))
        {
            error = "Lua script did not return boolean success value";
            lua_pop(L, 1);
            return false;
        }
        bool success = lua_toboolean(L, -1);
        lua_pop(L, 1);

        return success;
    }
};

LuaManager &LuaManager::instance()
{
    static LuaManager instance;
    return instance;
}

LuaManager::LuaManager()
    : impl_(new LuaManagerImpl()) {}

LuaManager::~LuaManager()
{
    delete impl_;
}

bool LuaManager::runScript(const std::string &scriptPath, Entity &entity,
                           const std::unordered_map<std::string, std::string> &params,
                           std::string &error)
{
    return impl_->runScript(scriptPath, entity, params, error);
}
