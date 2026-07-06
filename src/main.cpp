/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <tengine/lua_runtime.hpp>
#include <tengine/utils/logger.hpp>

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

    lua.bind("myprint", [](std::string msg) { std::println("tengine:Lua:> {}", msg); });
    lua.bind("strEqual", [](const std::string a, const std::string b) -> bool { return a == b; });

    lua.runFile("assets/scripts/init.lua");
    lua.runFile("assets/scripts/hello.lua");
    lua.call("hello", "Tetriarch", "Nice to know you");
    lua.runString(R"( require("utils").log("Direct Lua string execution"))");
    std::string addition = lua.callRet<std::string>("add", "a", "b");
    std::println("{}", addition);
}
