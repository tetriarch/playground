/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <stb/stb_image.h>

#include <tengine/asset_manager.hpp>
#include <tengine/ns/node_3D.hpp>
#include <tengine/ns/scene_tree.hpp>
#include <tengine/script_system.hpp>
#include <tengine/transform.hpp>
#include <tengine/utils/logger.hpp>

using namespace tengine;

const f32 DELTA_TIME = 0.01f;

const u32 WINDOW_WIDTH = 1280;
const u32 WINDOW_HEIGHT = 720;

struct Vertex {
    f32 position[3];
    f32 color[4];
    f32 uv[2];
};

std::vector<Vertex> quad{
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
};

std::vector<u32> indices{0, 1, 2, 2, 3, 0};
u32 indexCount = indices.size();

struct Transforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

f32 g_aspectRatio = static_cast<f32>(WINDOW_WIDTH) / static_cast<f32>(WINDOW_HEIGHT);
f32 g_orthoScale = 10.0f;
f32 g_viewportW = g_orthoScale * g_aspectRatio;
f32 g_viewportH = g_orthoScale;
f32 g_viewportWH = g_viewportW * 0.5f;
f32 g_viewportHH = g_viewportH * 0.5f;

glm::mat4 projMatrix =
    glm::orthoLH_ZO(-g_viewportWH, g_viewportWH, -g_viewportHH, g_viewportHH, 0.1f, 10.0f);

glm::mat4 viewMatrix = glm::mat4(1.0f);

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

    SDL_SetGPUSwapchainParameters(
        gpuDevice, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR, SDL_GPU_PRESENTMODE_VSYNC
    );

    bool running = true;
    SDL_Event event;

    auto am = AssetManager::get();
    am->setAssetRoot("assets");
    auto scriptSys = tengine::ScriptSystem::get();
    auto result = scriptSys->runScript("print(\"Welcome to TEngine\\n\")");
    if(!result) {
        LOG_ERROR("main", "failed to execute script");
    }

    // -------------------------------------------------------------------------------------- assets
    auto rawImageData = am->loadRaw("textures/T_pawn_C.png");
    if(!rawImageData) {
        LOG_ERROR("TextureLoader", "failed to load raw image data");
    }
    s32 imageWidth, imageHeight, imageChannelCount;
    u8* imageData = stbi_load_from_memory(
        rawImageData->data(), rawImageData->size(), &imageWidth, &imageHeight, &imageChannelCount, 0
    );
    if(!imageData) {
        LOG_ERROR("TextureLoader", "failed to load image data");
    }

    // ------------------------------------------------------------------------------- renderer init

    // --- vertex shader ---
    auto vsCode = am->loadRaw("shaders/unlit_color_vert.spv");
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
        .num_uniform_buffers = 1
    };
    SDL_GPUShader* vertexShader = SDL_CreateGPUShader(gpuDevice, &vertexShaderInfo);
    if(!vertexShader) {
        LOG_ERROR("Shader", "{}", SDL_GetError());
    }
    vsCode->clear();

    // -- fragment shader ---
    auto fsCode = am->loadRaw("shaders/unlit_color_frag.spv");
    if(!fsCode) {
        LOG_ERROR("AssetManager", "failed to load vertexShader");
    }

    SDL_GPUShaderCreateInfo fragmentShaderInfo = {
        .code_size = fsCode->size(),
        .code = fsCode->data(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .num_samplers = 1,
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
         .offset = offsetof(Vertex, color)},
        {.location = 2,
         .buffer_slot = 0,
         .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
         .offset = offsetof(Vertex, uv)}
    };

    SDL_GPUVertexInputState vertexInputState = {
        .vertex_buffer_descriptions = vertexBufferDescriptions,
        .num_vertex_buffers = 1,
        .vertex_attributes = vertexAttributes,
        .num_vertex_attributes = 3
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
        .rasterizer_state = SDL_GPURasterizerState{.enable_depth_clip = true},
        .target_info = pipelineTargetInfo,
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

    // --- texture setup ---
    SDL_GPUTextureCreateInfo textureCreateInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = static_cast<u32>(imageWidth),
        .height = static_cast<u32>(imageHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(gpuDevice, &textureCreateInfo);
    if(!texture) {
        LOG_ERROR("Texture", "{}", SDL_GetError());
    }

    SDL_SetGPUTextureName(gpuDevice, texture, "hello_world");

    SDL_GPUTransferBufferCreateInfo textureTransferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<u32>(imageWidth * imageHeight * imageChannelCount)
    };

    SDL_GPUTransferBuffer* textureTransferBuffer =
        SDL_CreateGPUTransferBuffer(gpuDevice, &textureTransferBufferInfo);
    if(!textureTransferBuffer) {
        LOG_ERROR("Texture", "{}", SDL_GetError());
    }

    u8* textureData = (u8*)SDL_MapGPUTransferBuffer(gpuDevice, textureTransferBuffer, false);
    if(!textureData) {
        LOG_ERROR("Texture", "{}", SDL_GetError());
    }
    memcpy(textureData, imageData, imageWidth * imageHeight * imageChannelCount);
    SDL_UnmapGPUTransferBuffer(gpuDevice, textureTransferBuffer);

    SDL_GPUSamplerCreateInfo textureSamplerCreateInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    };

    SDL_GPUSampler* textureSampler = SDL_CreateGPUSampler(gpuDevice, &textureSamplerCreateInfo);
    if(!textureSampler) {
        LOG_ERROR("Texture", "{}", SDL_GetError());
    }

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

    // where is the data
    SDL_GPUTextureTransferInfo textureTransferInfo = {
        .transfer_buffer = textureTransferBuffer,
        .offset = 0,
    };

    // where to upload the data
    SDL_GPUTextureRegion textureBufferRegion = {
        .texture = texture,
        .w = static_cast<u32>(imageWidth),
        .h = static_cast<u32>(imageHeight),
        .d = 1
    };

    SDL_UploadToGPUBuffer(copyPass, &vertexTransferBufferLocation, &vertexBufferRegion, false);
    SDL_UploadToGPUBuffer(copyPass, &indexTransferBufferLocation, &indexBufferRegion, false);
    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureBufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    SDL_ReleaseGPUTransferBuffer(gpuDevice, textureTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(gpuDevice, indexTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(gpuDevice, vertexTransferBuffer);
    rawImageData->clear();
    stbi_image_free(imageData);
    indices.clear();
    quad.clear();

    // ---------------------------------------------------------------------------------- scene init
    SceneTree tree;

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
            SDL_GPUColorTargetInfo colorTargetInfo = {
                .texture = swapchainTexture,
                .clear_color = {0.10f, 0.10f, 0.10f, 1.0f},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE
            };

            SDL_GPURenderPass* renderPass =
                SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
            SDL_BindGPUGraphicsPipeline(renderPass, graphicsPipeline);

            // transforms uniform push
            auto t = player->transform();
            Transforms transforms = {.model = t.toMatrix(), .view = viewMatrix, .proj = projMatrix};
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &transforms, sizeof(transforms));

            SDL_GPUBufferBinding vertexBufferBindings[] = {{.buffer = vertexBuffer, .offset = 0}};
            SDL_GPUBufferBinding indexBufferBindings[] = {{.buffer = indexBuffer, .offset = 0}};
            SDL_GPUTextureSamplerBinding textureSamplerBindings[] = {
                {.texture = texture, .sampler = textureSampler}
            };
            SDL_BindGPUVertexBuffers(renderPass, 0, vertexBufferBindings, 1);
            SDL_BindGPUIndexBuffer(renderPass, indexBufferBindings, SDL_GPU_INDEXELEMENTSIZE_32BIT);
            SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBindings, 1);
            SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
            SDL_EndGPURenderPass(renderPass);
        }

        if(!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
            LOG_FATAL("Renderer", "{}", SDL_GetError());
        }
    }

    // ------------------------------------------------------------------------------------- cleanup

    SDL_ReleaseGPUTexture(gpuDevice, texture);
    SDL_ReleaseGPUBuffer(gpuDevice, indexBuffer);
    SDL_ReleaseGPUBuffer(gpuDevice, vertexBuffer);
    SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
