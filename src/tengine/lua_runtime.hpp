/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#pragma once

#include <cstdarg>
#include <lua.hpp>
#include <tengine/aliases.hpp>
#include <tengine/utils/logger.hpp>

namespace tengine {

/**
 * @class LuaRuntime
 * @brief Holds Lua state and provides tools to execute Lua scripts, bind functions and read data
 * from scripts
 */
class LuaRuntime {
public:
    LuaRuntime();
    ~LuaRuntime();
    LuaRuntime(const LuaRuntime&) = delete;
    LuaRuntime& operator=(const LuaRuntime&) = delete;

public:
    /**
     * @brief Opens Lua libraries
     */
    void openLibs() const;

    /**
     * @brief runs Lua script from file
     *
     * @param scriptPath
     */
    void runFile(const std::string& scriptPath) const;

    /**
     * @brief runs Lua script directly from string
     *
     * @param string
     */
    void runString(const std::string& scriptString) const;

    /**
     * @brief Binds static function to be used in Lua
     * the format of the function has to be int fn(lua_State* L)
     *
     * @param functionName name by which is the function going to be called in Lua
     * @param function function to bind
     */
    void bindFunction(const std::string& functionName, lua_CFunction function) const;

    /**
     * @brief Calls Lua function from script
     *
     * @param funcitonName
     * @param args arguments to pass down to the Lua function
     */
    template <typename... TArgs>
    void call(const std::string& functionName, const TArgs&... args) const;

    /**
     * @brief Calls Lua function from script with return value
     *
     * @tparam TArgs
     * @param funcitonName
     * @param args arguments to pass down to the Lua function
     * @return value of type T
     */
    template <typename T, typename... TArgs>
        requires(!std::is_void_v<T>)
    T callRet(const std::string& funcitonName, const TArgs&... args) const;

    /**
     * @brief Pushes values to Lua stack to be used by Lua script
     *
     * @tparam T can be int, float, bool, const char*
     * @param value
     */
    template <typename T>
    void push(const T value) const;

    /**
     * @brief Pulles value from Lua stack. Poping value from the stack is responsibility of caller
     * of this function.
     *
     * @tparam T return type
     * @return T value
     */
    template <typename T>
    T pull() const;

private:
    lua_State* L_;
};

template <typename... TArgs>
inline void LuaRuntime::call(const std::string& functionName, const TArgs&... args) const {
    lua_getglobal(L_, functionName.c_str());
    if(!lua_isfunction(L_, -1)) {
        LOG_FATAL("Lua", "'{}' is not a function", functionName);
    }

    (push(args), ...);

    if(lua_pcall(L_, sizeof...(args), 0, 0)) {
        LOG_FATAL("Lua", "{}", lua_tostring(L_, -1));
    }
}

template <typename T, typename... TArgs>
    requires(!std::is_void_v<T>)
inline T LuaRuntime::callRet(const std::string& functionName, const TArgs&... args) const {
    lua_getglobal(L_, functionName.c_str());
    if(!lua_isfunction(L_, -1)) {
        LOG_FATAL("Lua", "'{}' is not a function", functionName);
    }

    (push(args), ...);

    if(lua_pcall(L_, sizeof...(args), 1, 0)) {
        LOG_FATAL("Lua", "{}", lua_tostring(L_, -1));
    }

    T v = pull<T>();
    lua_pop(L_, 1);
    return v;
}

template <>
inline void LuaRuntime::push(s32 value) const {
    lua_pushinteger(L_, value);
}

template <>
inline void LuaRuntime::push(f32 value) const {
    lua_pushnumber(L_, value);
}

template <>
inline void LuaRuntime::push(bool value) const {
    lua_pushboolean(L_, value);
}

template <>
inline void LuaRuntime::push(const char* value) const {
    lua_pushstring(L_, value);
}

template <>
inline void LuaRuntime::push(const std::string& value) const {
    lua_pushstring(L_, value.c_str());
}

template <>
inline s32 LuaRuntime::pull() const {
    if(!lua_isinteger(L_, -1)) {
        LOG_FATAL("Lua", "expected integer value, got {}", luaL_typename(L_, -1));
    }
    return static_cast<s32>(lua_tointeger(L_, -1));
}

template <>
inline f32 LuaRuntime::pull() const {
    if(!lua_isnumber(L_, -1)) {
        LOG_FATAL("Lua", "expected number value, got {}", luaL_typename(L_, -1));
    }
    return static_cast<f32>(lua_tonumber(L_, -1));
}

template <>
inline bool LuaRuntime::pull() const {
    if(!lua_isboolean(L_, -1)) {
        LOG_FATAL("Lua", "expected bool value, got {}", luaL_typename(L_, -1));
    }
    return static_cast<bool>(lua_toboolean(L_, -1));
}

template <>
inline std::string LuaRuntime::pull() const {
    if(lua_type(L_, -1) != LUA_TSTRING) {
        LOG_FATAL("Lua", "expected string value, got {}", luaL_typename(L_, -1));
    }
    return static_cast<std::string>(lua_tostring(L_, -1));
}

}  // namespace tengine
