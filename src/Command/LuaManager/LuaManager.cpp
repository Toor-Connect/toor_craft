#include "LuaManager.h"
#include "EntityManager.h"
#include "FileSystemFactory.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
#include <unordered_map>
#include <regex>
#include <sstream>
#include <filesystem>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

class LuaManager::LuaManagerImpl
{
public:
    lua_State *L;
    std::unique_ptr<FileSystemInterface> fs_;

    LuaManagerImpl()
        : fs_(createFileSystem())
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

    // ---------- JSON ↔ Lua Helpers ----------
    static void pushJsonToLua(lua_State *L, const nlohmann::json &j)
    {
        if (j.is_object())
        {
            lua_newtable(L);
            for (auto it = j.begin(); it != j.end(); ++it)
            {
                lua_pushstring(L, it.key().c_str());
                pushJsonToLua(L, it.value());
                lua_settable(L, -3);
            }
        }
        else if (j.is_array())
        {
            lua_newtable(L);
            int index = 1;
            for (auto &el : j)
            {
                lua_pushinteger(L, index++);
                pushJsonToLua(L, el);
                lua_settable(L, -3);
            }
        }
        else if (j.is_string())
        {
            lua_pushstring(L, j.get<std::string>().c_str());
        }
        else if (j.is_number_integer())
        {
            lua_pushinteger(L, j.get<lua_Integer>());
        }
        else if (j.is_number_float())
        {
            lua_pushnumber(L, j.get<lua_Number>());
        }
        else if (j.is_boolean())
        {
            lua_pushboolean(L, j.get<bool>());
        }
        else if (j.is_null())
        {
            lua_pushnil(L);
        }
    }

    static nlohmann::json luaToJson(lua_State *L, int index)
    {
        nlohmann::json j;
        if (lua_istable(L, index))
        {
            bool isArray = true;
            lua_pushnil(L);
            while (lua_next(L, index) != 0)
            {
                if (!lua_isinteger(L, -2))
                {
                    isArray = false;
                }
                lua_pop(L, 1);
            }
            lua_pushnil(L);
            if (isArray)
            {
                int maxIndex = (int)lua_rawlen(L, index);
                for (int i = 1; i <= maxIndex; i++)
                {
                    lua_rawgeti(L, index, i);
                    j.push_back(luaToJson(L, -1));
                    lua_pop(L, 1);
                }
            }
            else
            {
                while (lua_next(L, index) != 0)
                {
                    std::string key = lua_tostring(L, -2);
                    j[key] = luaToJson(L, -1);
                    lua_pop(L, 1);
                }
            }
        }
        else if (lua_isstring(L, index))
        {
            j = lua_tostring(L, index);
        }
        else if (lua_isboolean(L, index))
        {
            j = (bool)lua_toboolean(L, index);
        }
        else if (lua_isinteger(L, index))
        {
            j = (lua_Integer)lua_tointeger(L, index);
        }
        else if (lua_isnumber(L, index))
        {
            j = (lua_Number)lua_tonumber(L, index);
        }
        else if (lua_isnil(L, index))
        {
            j = nullptr;
        }
        return j;
    }

    // ---------- Lua Bindings ----------
    static int lua_jsonDecode(lua_State *L)
    {
        const char *jsonStr = luaL_checkstring(L, 1);
        try
        {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            pushJsonToLua(L, j);
            return 1;
        }
        catch (const std::exception &e)
        {
            lua_pushnil(L);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    static int lua_jsonEncode(lua_State *L)
    {
        if (!lua_istable(L, 1))
        {
            return luaL_error(L, "Expected a table for json.encode");
        }
        try
        {
            nlohmann::json j = luaToJson(L, 1);
            std::string jsonStr = j.dump();
            lua_pushstring(L, jsonStr.c_str());
            return 1;
        }
        catch (const std::exception &e)
        {
            lua_pushnil(L);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    static int lua_generateDocumentation(lua_State *L)
    {
        const char *templatePathC = luaL_checkstring(L, 1);
        const char *outputPathC = luaL_checkstring(L, 2);

        if (!lua_istable(L, 3))
        {
            return luaL_error(L, "Third argument must be a table");
        }

        try
        {
            LuaManager &manager = LuaManager::instance();

            std::string templateContent;
            manager.impl_->fs_->readFile(templatePathC, templateContent);

            nlohmann::json jsonData = luaToJson(L, 3);
            inja::Environment env;
            std::string rendered = env.render(templateContent, jsonData);

            manager.impl_->fs_->writeFile(outputPathC, rendered);

            lua_pushboolean(L, true);
            lua_pushstring(L, "");
            return 2;
        }
        catch (const std::exception &e)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    static int lua_getField(lua_State *L)
    {
        const char *entityId = luaL_checkstring(L, 1);
        const char *fieldName = luaL_checkstring(L, 2);

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

        lua_pushstring(L, fieldValue->toString().c_str());
        return 1;
    }

    static int lua_getDict(lua_State *L)
    {
        const char *entityId = luaL_checkstring(L, 1);
        Entity *entity = EntityManager::instance().getEntityById(entityId);
        if (!entity)
        {
            lua_pushnil(L);
            return 1;
        }
        auto dict = entity->getDict();
        nlohmann::json j(dict);
        pushJsonToLua(L, j);
        return 1;
    }

    static int lua_regexMatch(lua_State *L)
    {
        const char *pattern = luaL_checkstring(L, 1);
        const char *input = luaL_checkstring(L, 2);
        try
        {
            std::regex re(pattern);
            bool matched = std::regex_match(input, re);
            lua_pushboolean(L, matched);
        }
        catch (const std::regex_error &)
        {
            lua_pushboolean(L, false);
        }
        return 1;
    }

    static int lua_writeFile(lua_State *L)
    {
        try
        {
            const char *filepath = luaL_checkstring(L, 1);
            const char *content = luaL_checkstring(L, 2);

            LuaManager &manager = LuaManager::instance();

            manager.impl_->fs_->writeFile(filepath, content);

            lua_pushboolean(L, true);
            lua_pushstring(L, "");
            return 2;
        }
        catch (const std::exception &e)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    static int lua_readFile(lua_State *L)
    {
        try
        {
            const char *filepath = luaL_checkstring(L, 1);
            std::string content;
            LuaManager &manager = LuaManager::instance();
            manager.impl_->fs_->readFile(filepath, content);

            lua_pushboolean(L, true);
            lua_pushstring(L, content.c_str());
            return 1;
        }
        catch (const std::exception &e)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    void registerLuaFunctions()
    {
        lua_register(L, "getField", lua_getField);
        lua_register(L, "regexMatch", lua_regexMatch);
        lua_register(L, "writeFile", lua_writeFile);
        lua_register(L, "readFile", lua_readFile);
        lua_register(L, "generateDocumentation", lua_generateDocumentation);
        lua_register(L, "getDict", lua_getDict);
        lua_register(L, "json_decode", lua_jsonDecode);
        lua_register(L, "json_encode", lua_jsonEncode);
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

    void runScript(const std::string &scriptPath,
                   const Entity &entity,
                   const std::unordered_map<std::string, std::string> &params)
    {
        if (!L)
        {
            throw std::runtime_error("Lua state not initialized");
        }

        LuaManager &manager = LuaManager::instance();
        std::string scriptContent;
        manager.impl_->fs_->readFile(scriptPath, scriptContent);

        struct LuaStackGuard
        {
            lua_State *L;
            int top;
            LuaStackGuard(lua_State *state) : L(state), top(lua_gettop(state)) {}
            ~LuaStackGuard() { lua_settop(L, top); }
        } guard(L);

        int loadStatus = luaL_loadbuffer(L, scriptContent.c_str(), scriptContent.size(), scriptPath.c_str());

        if (loadStatus != LUA_OK)
        {
            std::string err = lua_tostring(L, -1);
            throw std::runtime_error("[LuaManager] Failed to load script '" + scriptPath + "': " + err);
        }

        pushEntity(entity);
        pushParamsTable(params);

        int callStatus = lua_pcall(L, 2, 2, 0);
        if (callStatus != LUA_OK)
        {
            std::string err = lua_tostring(L, -1);
            throw std::runtime_error("[LuaManager] Lua runtime error in '" + scriptPath + "': " + err);
        }

        if (!lua_isboolean(L, -1))
        {
            throw std::runtime_error("Lua script '" + scriptPath + "' must return a boolean as the first value");
        }

        bool success = lua_toboolean(L, -1);

        if (success)
        {
            return;
        }
        lua_pop(L, 1);

        if (!lua_isstring(L, -1))
        {
            throw std::runtime_error("Lua script '" + scriptPath + "' failed but did not return an error message");
        }

        std::string errorMsg = lua_tostring(L, -1);
        throw std::runtime_error("[LuaManager] Script '" + scriptPath + "' failed: " + errorMsg);
    }
};

// ------- LuaManager Singleton API --------
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

void LuaManager::runScript(const std::string &scriptPath, const Entity &entity,
                           const std::unordered_map<std::string, std::string> &params)
{
    return impl_->runScript(scriptPath, entity, params);
}
