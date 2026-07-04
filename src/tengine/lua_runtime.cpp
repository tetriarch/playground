/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include <tengine/lua_runtime.hpp>

namespace tengine {

LuaRuntime::LuaRuntime() : L_(nullptr) {
    L_ = luaL_newstate();
    if(!L_) {
        LOG_FATAL("Lua", "failed to create Lua state");
    }
}

LuaRuntime::~LuaRuntime() {
    if(L_) {
        lua_close(L_);
    }
}

void LuaRuntime::openLibs() const {
    // TODO: Might wanna specify which ones do we wanna open
    luaL_openlibs(L_);
}

void LuaRuntime::runFile(const std::string& scriptPath) const {
    if(luaL_dofile(L_, scriptPath.c_str()) != LUA_OK) {
        LOG_FATAL("Lua", "{}", lua_tostring(L_, -1));
    }
}

void LuaRuntime::runString(const std::string& scriptString) const {
    if(luaL_dostring(L_, scriptString.c_str()) != LUA_OK) {
        LOG_FATAL("Lua", "{}", lua_tostring(L_, -1));
    }
}

void LuaRuntime::bindFunction(const std::string& functionName, lua_CFunction function) const {
    lua_pushcfunction(L_, function);
    lua_setglobal(L_, functionName.c_str());
}

}  // namespace tengine
