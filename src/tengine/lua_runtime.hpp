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
     * @param args arguments of the function to pass
     */
    template <typename... TArgs>
    void call(const std::string& funcitonName, const TArgs&... args) const;

    /**
     * @brief Pushes values to Lua stack to be used by Lua script
     *
     * @tparam T can be int, float, bool, const char*
     * @param value
     */
    template <typename T>
    void push(const T value) const;

private:
    lua_State* L_;
};

template <>
inline void LuaRuntime::push(const s32& value) const {
    lua_pushinteger(L_, value);
}

template <>
inline void LuaRuntime::push(const f32& value) const {
    lua_pushnumber(L_, value);
}

template <>
inline void LuaRuntime::push(const bool& value) const {
    lua_pushboolean(L_, value);
}

template <>
inline void LuaRuntime::push(const char* value) const {
    lua_pushstring(L_, value);
}

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

}  // namespace tengine
