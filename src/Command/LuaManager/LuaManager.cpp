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
#include <regex>
#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

class LuaManager::LuaManagerImpl
{
public:
    lua_State *L;
    fs::path baseDirectory_; // Base dir for relative paths

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

    void setBaseDirectory(const std::string& baseDir)
    {
        baseDirectory_ = fs::path(baseDir);
    }

    // Resolve a path: if absolute, keep as is; if relative, join with baseDirectory_
    fs::path resolvePath(const std::string& path)
    {
        fs::path p(path);
        if (p.is_absolute())
            return p;
        if (baseDirectory_.empty())
            throw std::runtime_error("Base directory for LuaManager not set");
        return baseDirectory_ / p;
    }

    // Lua callable function: getField(entityId, fieldName)
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
        const char *filepath = luaL_checkstring(L, 1);
        const char *content = luaL_checkstring(L, 2);

        LuaManager& manager = LuaManager::instance();

        try
        {
            fs::path fullPath = manager.impl_->resolvePath(filepath);

            std::ofstream ofs(fullPath);
            if (!ofs)
            {
                lua_pushboolean(L, false);
                lua_pushstring(L, "Failed to open file for writing");
                return 2;
            }
            ofs << content;
            ofs.close();

            lua_pushboolean(L, true);
            lua_pushstring(L, "");
            return 2;
        }
        catch (const std::exception& e)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    static int lua_readFile(lua_State *L)
    {
        const char *filepath = luaL_checkstring(L, 1);
        LuaManager& manager = LuaManager::instance();

        try
        {
            fs::path fullPath = manager.impl_->resolvePath(filepath);

            std::ifstream ifs(fullPath);
            if (!ifs)
            {
                lua_pushnil(L);
                lua_pushstring(L, "Failed to open file for reading");
                return 2;
            }
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string content = buffer.str();

            lua_pushstring(L, content.c_str());
            lua_pushstring(L, "");
            return 2;
        }
        catch (const std::exception& e)
        {
            lua_pushnil(L);
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

        // Expect Lua script returns (bool success, string error)
        int callStatus = lua_pcall(L, 2, 2, 0);
        if (callStatus != LUA_OK)
        {
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }

        if (!lua_isstring(L, -1))
        {
            error = "Lua script did not return error message string";
            lua_pop(L, 2);
            return false;
        }
        error = lua_tostring(L, -1);
        lua_pop(L, 1);

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
    : impl_(new LuaManagerImpl())
{
}

LuaManager::~LuaManager()
{
    delete impl_;
}

void LuaManager::setBaseDirectory(const std::string &baseDir)
{
    impl_->setBaseDirectory(baseDir);
}

bool LuaManager::runScript(const std::string &scriptPath, Entity &entity,
                           const std::unordered_map<std::string, std::string> &params,
                           std::string &error)
{
    return impl_->runScript(scriptPath, entity, params, error);
}
