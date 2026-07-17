/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <tengine/asset_manager.hpp>
#include <tengine/ns/node_3D.hpp>
#include <tengine/ns/node_tree.hpp>
#include <tengine/script_system.hpp>
#include <tengine/utils/logger.hpp>

using namespace tengine;

int main(int argc, char** argv) {
    setLogger(std::make_shared<tengine::Logger>());
    LOGGER->addSink(std::make_shared<tengine::ConsoleLogSink>());
#ifdef DEBUG
    LOGGER->setLogLevel(LogLevel::Trace);
#else
    LOGGER->setLogLevel(LogLevel::Info);
#endif
    LOGGER->setColoredOutput(true);

    AssetManager::get()->setAssetRoot("assets");
    auto scriptSys = tengine::ScriptSystem::get();
    scriptSys->runScript("print(\"Welcome to TEngine\")");

    // ---------------------------------------------------------------------------------------- init

    // this is a separate scene setup as if were loading a scene from file
    auto world = std::make_shared<Node>("Scene1");
    auto player = std::make_shared<Node3D>("Player");
    auto sword = std::make_shared<Node3D>("Sword");
    auto shield = std::make_shared<Node3D>("Shield");
    world->addChild(player);
    player->addChild(sword);
    player->addChild(shield);
    player->setScriptPath("scripts/player.lua");
    player->load();

    // ---------------------------------------------------------------------------------------- loop

    // bool running = false;
    // do {
    // } while(running);
    //
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
