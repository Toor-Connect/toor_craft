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
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
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

    void setBaseDirectory(const std::string &baseDir)
    {
        baseDirectory_ = fs::path(baseDir);
    }

    // Resolve a path: if absolute, keep as is; if relative, join with baseDirectory_
    fs::path resolvePath(const std::string &path)
    {
        fs::path p(path);
        if (p.is_absolute())
            return p;
        if (baseDirectory_.empty())
            throw std::runtime_error("Base directory for LuaManager not set");
        return baseDirectory_ / p;
    }

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
            int index = 1; // Lua uses 1-based indexing
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
            // Detect if table is array-like or object-like
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
                // Array mode
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
                // Object mode
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

    static int lua_jsonDecode(lua_State *L)
    {
        const char *jsonStr = luaL_checkstring(L, 1);
        try
        {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            pushJsonToLua(L, j);
            return 1; // return one table
        }
        catch (const std::exception &e)
        {
            lua_pushnil(L);
            lua_pushstring(L, e.what());
            return 2; // return nil + error
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

            fs::path templatePath = manager.impl_->resolvePath(templatePathC);
            fs::path outputPath = manager.impl_->resolvePath(outputPathC);

            // Load template file
            std::ifstream templateFile(templatePath);
            if (!templateFile)
            {
                lua_pushboolean(L, false);
                lua_pushfstring(L, "Failed to open template file: %s", templatePath.string().c_str());
                return 2;
            }
            std::stringstream buffer;
            buffer << templateFile.rdbuf();
            std::string templateContent = buffer.str();

            //  Convert Lua table (argument #3) into JSON
            nlohmann::json jsonData = luaToJson(L, 3);

            // Render using inja
            inja::Environment env;
            std::string rendered = env.render(templateContent, jsonData);

            // Write output
            std::ofstream outputFile(outputPath);
            if (!outputFile)
            {
                lua_pushboolean(L, false);
                lua_pushfstring(L, "Failed to open output file: %s", outputPath.string().c_str());
                return 2;
            }
            outputFile << rendered;

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

        return 1; // One return value (the Lua table)
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

        LuaManager &manager = LuaManager::instance();

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
        catch (const std::exception &e)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, e.what());
            return 2;
        }
    }

    static int lua_readFile(lua_State *L)
    {
        const char *filepath = luaL_checkstring(L, 1);
        LuaManager &manager = LuaManager::instance();

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
        catch (const std::exception &e)
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
            std::cerr << "[LuaManager] Failed to load script: " << lua_tostring(L, -1) << std::endl;
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }
        else
        {
            std::cout << "[LuaManager] Script loaded successfully: " << scriptPath << std::endl;
        }

        pushEntity(entity);
        pushParamsTable(params);

        // Expect Lua script returns (bool success, string error)
        int callStatus = lua_pcall(L, 2, 2, 0);
        if (callStatus != LUA_OK)
        {
            std::cerr << "[LuaManager] Lua runtime error: " << lua_tostring(L, -1) << std::endl;
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
