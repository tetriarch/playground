/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <tengine/lua_runtime.hpp>
#include <tengine/utils/logger.hpp>

// l_print exposure to be called from Lua
int l_print(lua_State* L) {
    std::string s = luaL_checkstring(L, 1);  // gets to string convertible value
    std::println("{}", s);
    return 0;  // we return 0 because we don't push result to stack
}

int main(int argc, char** argv) {
    tengine::setLogger(std::make_shared<tengine::Logger>());
    LOGGER->addSink(std::make_shared<tengine::ConsoleLogSink>());
#ifdef DEBUG
    LOGGER->setLogLevel(LogLevel::Trace);
#else
    LOGGER->setLogLevel(LogLevel::Info);
#endif
    LOGGER->setColoredOutput(true);

    tengine::LuaRuntime lua;
    lua.openLibs();
    lua.bindFunction("myprint", l_print);
    lua.runFile("assets/scripts/hello.lua");
    lua.call("hello", "Tetriarch", "Nice to know you");
    lua.runString(R"( print("Direct Lua string execution"))");
}
