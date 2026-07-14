/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <tengine/asset_manager.hpp>
#include <tengine/ns/node_tree.hpp>
#include <tengine/script_system.hpp>
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

    tengine::AssetManager::get()->setAssetRoot("assets");
    auto scriptSys = tengine::ScriptSystem::get();
    tengine::ScriptInstance luaHandle = scriptSys->runFile("scripts/init.lua");
    scriptSys->runScript("print(\"Hello, World\")");

    // lua.set_function("myprint", [](std::string msg) { std::println("tengine:Lua:> {}", msg); });
    // lua.set_function("strEqual", [](const std::string& a, const std::string& b) -> bool {
    //     return a == b;
    // });

    // lua["hello"]("Tetriarch", "nice to know you");
    // lua.script(R"( require("utils").log("Direct Lua string execution"))");

    // log has to exist and has to be a function
    // std::string fnName = "log";
    // auto log = luaHandle[fnName];
    // TENGINE_ASSERT(log != sol::nil, "luaHandle has no object named {}", fnName);
    // TENGINE_ASSERT(log.get_type() == sol::type::function, "{} is not a function", fnName);
    //
    // log("table test");
    //
    // std::string addition = lua["add"]("a", "b");
    // std::println("{}", addition);

    // we start here
    // tengine::NodeTree nodeTree;
    // tengine::Node3D node;
    // auto transform = node.transform();
    // transform.position.x = 10.f;
    // node.setTransform(transform);
    // node.ready();
}
