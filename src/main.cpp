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

struct Vertex {
    f32 position[3];
    f32 color[4];
};

std::vector<Vertex> quad{
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
};

std::vector<u32> indices{0, 1, 2, 2, 3, 0};

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

    // --------------------------------------------------------------------------------- system init
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_FATAL("Core", "{}", SDL_GetError());
    }

    SDL_Window* window =
        SDL_CreateWindow("playground", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
    if(!window) {
        LOG_FATAL("Window", "{}", SDL_GetError());
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_GPUDevice* gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");

    if(!gpuDevice) {
        LOG_FATAL("Renderer", "{}", SDL_GetError());
    }

    if(!SDL_ClaimWindowForGPUDevice(gpuDevice, window)) {
        LOG_FATAL("Renderer", "{}", SDL_GetError());
    }

    bool running = true;
    SDL_Event event;

    AssetManager::get()->setAssetRoot("assets");
    auto scriptSys = tengine::ScriptSystem::get();
    auto result = scriptSys->runScript("print(\"Welcome to TEngine\\n\")");
    if(!result) {
        LOG_ERROR("main", "failed to execute script");
    }

    // ------------------------------------------------------------------------------- renderer init

    // --- vertex shader ---
    auto vsCode = AssetManager::get()->loadRaw("shaders/unlit_color_vert.spv");
    if(!vsCode) {
        LOG_ERROR("AssetManager", "failed to load vertexShader");
    }

    SDL_GPUShaderCreateInfo vertexShaderInfo = {
        .code_size = vsCode->size(),
        .code = vsCode->data(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0
    };
    SDL_GPUShader* vertexShader = SDL_CreateGPUShader(gpuDevice, &vertexShaderInfo);
    if(!vertexShader) {
        LOG_ERROR("Shader", "{}", SDL_GetError());
    }
    vsCode->clear();

    // -- fragment shader ---
    auto fsCode = AssetManager::get()->loadRaw("shaders/unlit_color_frag.spv");
    if(!fsCode) {
        LOG_ERROR("AssetManager", "failed to load vertexShader");
    }

    SDL_GPUShaderCreateInfo fragmentShaderInfo = {
        .code_size = fsCode->size(),
        .code = fsCode->data(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0,
    };
    SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(gpuDevice, &fragmentShaderInfo);
    if(!fragmentShader) {
        LOG_ERROR("Shader", "{}", SDL_GetError());
    }
    fsCode->clear();

    SDL_GPUVertexBufferDescription vertexBufferDescriptions[] = {{
        .slot = 0,
        .pitch = sizeof(Vertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    }};

    SDL_GPUVertexAttribute vertexAttributes[] = {
        {.location = 0,
         .buffer_slot = 0,
         .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
         .offset = 0},
        {.location = 1,
         .buffer_slot = 0,
         .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
         .offset = offsetof(Vertex, color)}
    };

    SDL_GPUVertexInputState vertexInputState = {
        .vertex_buffer_descriptions = vertexBufferDescriptions,
        .num_vertex_buffers = 1,
        .vertex_attributes = vertexAttributes,
        .num_vertex_attributes = 2
    };

    SDL_GPUColorTargetBlendState colorTargetBlendState = {
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
        .enable_blend = true,
    };

    SDL_GPUColorTargetDescription colorTargetDescriptions[] = {
        {.format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window),
         .blend_state = colorTargetBlendState}
    };

    SDL_GPUGraphicsPipelineTargetInfo pipelineTargetInfo = {
        .color_target_descriptions = colorTargetDescriptions, .num_color_targets = 1
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = pipelineTargetInfo
    };

    SDL_GPUGraphicsPipeline* graphicsPipeline =
        SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineCreateInfo);
    if(!graphicsPipeline) {
        LOG_ERROR("Renderer", "{}", SDL_GetError());
    }
    SDL_ReleaseGPUShader(gpuDevice, fragmentShader);
    SDL_ReleaseGPUShader(gpuDevice, vertexShader);

    // --- vertex buffer setup ---
    SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = static_cast<u32>(quad.size() * sizeof(Vertex))
    };

    SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(gpuDevice, &vertexBufferInfo);
    if(!vertexBuffer) {
        LOG_ERROR("VertexBuffer", "{}", SDL_GetError());
    }

    SDL_GPUTransferBufferCreateInfo vertexTransferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<u32>(quad.size() * sizeof(Vertex))
    };

    SDL_GPUTransferBuffer* vertexTransferBuffer =
        SDL_CreateGPUTransferBuffer(gpuDevice, &vertexTransferBufferInfo);

    if(!vertexTransferBuffer) {
        LOG_ERROR("VertexBuffer", "{}", SDL_GetError());
    }

    Vertex* vertexData = (Vertex*)SDL_MapGPUTransferBuffer(gpuDevice, vertexTransferBuffer, false);
    if(!vertexData) {
        LOG_ERROR("VertexBuffer", "{}", SDL_GetError());
    }

    memcpy(vertexData, quad.data(), quad.size() * sizeof(Vertex));

    SDL_UnmapGPUTransferBuffer(gpuDevice, vertexTransferBuffer);

    // --- index buffer setup ---
    SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = static_cast<u32>(indices.size() * sizeof(u32))
    };

    SDL_GPUBuffer* indexBuffer = SDL_CreateGPUBuffer(gpuDevice, &indexBufferInfo);
    if(!indexBuffer) {
        LOG_ERROR("IndexBuffer", "{}", SDL_GetError());
    }

    SDL_GPUTransferBufferCreateInfo indexTransferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<u32>(indices.size() * sizeof(u32))
    };

    SDL_GPUTransferBuffer* indexTransferBuffer =
        SDL_CreateGPUTransferBuffer(gpuDevice, &indexTransferBufferInfo);
    if(!indexTransferBuffer) {
        LOG_ERROR("IndexBuffer", "{}", SDL_GetError());
    }

    u32* indexData = (u32*)SDL_MapGPUTransferBuffer(gpuDevice, indexTransferBuffer, false);
    if(!indexData) {
        LOG_ERROR("IndexBuffer", "{}", SDL_GetError());
    }
    memcpy(indexData, indices.data(), indices.size() * sizeof(u32));
    SDL_UnmapGPUTransferBuffer(gpuDevice, indexTransferBuffer);

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
    if(!commandBuffer) {
        LOG_FATAL("Renderer", "{}", SDL_GetError());
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    // where is the data
    SDL_GPUTransferBufferLocation vertexTransferBufferLocation = {
        .transfer_buffer = vertexTransferBuffer, .offset = 0
    };

    // where to upload the data
    SDL_GPUBufferRegion vertexBufferRegion = {
        .buffer = vertexBuffer, .offset = 0, .size = static_cast<u32>(quad.size() * sizeof(Vertex))
    };

    // where is the data
    SDL_GPUTransferBufferLocation indexTransferBufferLocation = {
        .transfer_buffer = indexTransferBuffer, .offset = 0
    };

    // where to upload the data
    SDL_GPUBufferRegion indexBufferRegion = {
        .buffer = indexBuffer, .offset = 0, .size = static_cast<u32>(indices.size() * sizeof(u32))
    };

    SDL_UploadToGPUBuffer(copyPass, &vertexTransferBufferLocation, &vertexBufferRegion, false);
    SDL_UploadToGPUBuffer(copyPass, &indexTransferBufferLocation, &indexBufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);

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

        if(swapchainTexture) {
            SDL_GPUColorTargetInfo colorTargetInfo = {};
            colorTargetInfo = {
                .texture = swapchainTexture,
                .clear_color = {0.10f, 0.10f, 0.10f, 1.0f},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE
            };

            SDL_GPURenderPass* renderPass =
                SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
            SDL_BindGPUGraphicsPipeline(renderPass, graphicsPipeline);

            SDL_GPUBufferBinding vertexBufferBindings[] = {{.buffer = vertexBuffer, .offset = 0}};
            SDL_GPUBufferBinding indexBufferBindings[] = {{.buffer = indexBuffer, .offset = 0}};
            SDL_BindGPUVertexBuffers(renderPass, 0, vertexBufferBindings, 1);
            SDL_BindGPUIndexBuffer(renderPass, indexBufferBindings, SDL_GPU_INDEXELEMENTSIZE_32BIT);

            SDL_DrawGPUIndexedPrimitives(renderPass, indices.size(), 1, 0, 0, 0);
            SDL_EndGPURenderPass(renderPass);
        }

        if(!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
            LOG_FATAL("Renderer", "{}", SDL_GetError());
        }
    }

    // ------------------------------------------------------------------------------------- cleanup

    SDL_ReleaseGPUTransferBuffer(gpuDevice, vertexTransferBuffer);
    SDL_ReleaseGPUBuffer(gpuDevice, vertexBuffer);
    SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
