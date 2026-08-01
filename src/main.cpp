/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <tengine/asset_manager.hpp>
#include <tengine/ns/node_3D.hpp>
#include <tengine/ns/scene_tree.hpp>
#include <tengine/script_system.hpp>
#include <tengine/utils/logger.hpp>

using namespace tengine;

const f32 DELTA_TIME = 0.01f;

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
    auto result = scriptSys->runScript("print(\"Welcome to TEngine\\n\")");
    if(!result) {
        LOG_ERROR("main", "failed to execute script");
    }

    // ---------------------------------------------------------------------------------------- init
    SceneTree tree;

    NodeHandle playerHandle;
    // this is a separate scene setup as if were loading a scene from file
    {
        auto world = std::make_shared<Node>("Scene1");
        auto player = std::make_shared<Node3D>("Player");
        auto sword = std::make_shared<Node3D>("Sword");
        world->addChild(player);
        player->addChild(sword);
        player->setScriptPath("scripts/player.lua");
        player->load();

        tree.setSceneRoot(world);
        playerHandle = player;
    }

    // ---------------------------------------------------------------------------------------- loop

    for(u32 x = 0; x < 2; x++) {
        tree.orderTreeUpdates();
        std::println("== Begin Frame ==");
        if(x == 0) {
            if(auto player = playerHandle.lock()) {
                player->addChild(std::make_shared<Node3D>("Shield"));
            }
        }
        tree.update(DELTA_TIME);

        tree.render();
        std::println("== End Frame ==");
    }
}
