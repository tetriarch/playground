/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <sol/sol.hpp>
#include <tengine/ns/node_tree.hpp>
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

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua.set_function("myprint", [](std::string msg) { std::println("tengine:Lua:> {}", msg); });
    lua.set_function("strEqual", [](const std::string& a, const std::string& b) -> bool {
        return a == b;
    });

    lua.do_file("assets/scripts/init.lua");

    // TODO: This might be worth to wrap
    auto result = lua.do_file("assets/scripts/hello.lua");
    if(result.status() != sol::call_status::ok) {
        sol::error err = result;
        std::println("{}", err.what());
    }

    lua["hello"]("Tetriarch", "Nice to know you");
    lua.script(R"( require("utils").log("Direct Lua string execution"))");
    std::string addition = lua["add"]("a", "b");
    std::println("{}", addition);

    // we start here
    tengine::NodeTree nodeTree;
}
