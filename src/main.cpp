/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <tengine/asset_manager.hpp>
#include <tengine/ns/node_3D.hpp>
#include <tengine/ns/scene_tree.hpp>
#include <tengine/script_system.hpp>
#include <tengine/utils/logger.hpp>

using namespace tengine;

const f32 DELTA_TIME = 0.01f;
const u32 WINDOW_WIDTH = 1280;
const u32 WINDOW_HEIGHT = 720;

int main(int argc, char** argv) {
    // --------------------------------------------------------------------------------- logger init
    setLogger(std::make_shared<tengine::Logger>());
    LOGGER->addSink(std::make_shared<tengine::ConsoleLogSink>());
#ifdef DEBUG
    LOGGER->setLogLevel(LogLevel::Trace);
#else
    LOGGER->setLogLevel(LogLevel::Info);
#endif
    LOGGER->setColoredOutput(true);

    // --------------------------------------------------------------------------------- engine init
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_FATAL("Core", "{}", SDL_GetError());
    }

    SDL_Window* window =
        SDL_CreateWindow("playground", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
    if(!window) {
        LOG_FATAL("Window", "{}", SDL_GetError());
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_GPUDevice* gpuDevice = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true,
        "vulkan"
    );
    if(!gpuDevice) {
        LOG_FATAL("Renderer", "{}", SDL_GetError());
    }

    if(!SDL_ClaimWindowForGPUDevice(gpuDevice, window)) {
        LOG_FATAL("Renderer", "{}", SDL_GetError());
    }

    SDL_SetGPUSwapchainParameters(
        gpuDevice, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE
    );

    bool running = true;
    SDL_Event event;

    AssetManager::get()->setAssetRoot("assets");
    auto scriptSys = tengine::ScriptSystem::get();
    auto result = scriptSys->runScript("print(\"Welcome to TEngine\\n\")");
    if(!result) {
        LOG_ERROR("main", "failed to execute script");
    }

    // ---------------------------------------------------------------------------------- scene init
    SceneTree tree;

    NodeHandle playerHandle;
    // this is a separate scene setup as if were loading a scene from file
    {
        auto world = std::make_shared<Node>("Scene1");
        auto player = std::make_shared<Node3D>("Player");
        auto sword = std::make_shared<Node3D>("Sword");
        auto shield = std::make_shared<Node3D>("Shield");
        world->addChild(player);
        player->addChild(sword);
        player->addChild(shield);
        player->setScriptPath("scripts/player.lua");
        player->load();

        tree.setSceneRoot(world);
        playerHandle = player;
    }

    // ---------------------------------------------------------------------------------------- loop
    while(running) {
        // --- events ---
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        // --- update ---
        tree.orderTreeUpdates();
        tree.update(DELTA_TIME);

        // --- render ---
        tree.render();
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
        if(!commandBuffer) {
            LOG_FATAL("Renderer", "{}", SDL_GetError());
        }

        SDL_GPUTexture* swapchainTexture;
        if(!SDL_WaitAndAcquireGPUSwapchainTexture(
               commandBuffer, window, &swapchainTexture, nullptr, nullptr
           )) {
            LOG_FATAL("Renderer", "{}", SDL_GetError());
        }

        if(swapchainTexture != nullptr) {
            SDL_GPUColorTargetInfo colorTargetInfo = {};
            colorTargetInfo = {
                .texture = swapchainTexture,
                .clear_color = {0.10f, 0.10f, 0.10f, 1.0f},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE
            };

            SDL_GPURenderPass* renderPass =
                SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
            SDL_EndGPURenderPass(renderPass);
        }

        if(!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
            LOG_FATAL("Renderer", "{}", SDL_GetError());
        }
    }

    SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyWindow(window);
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_Quit();
}
