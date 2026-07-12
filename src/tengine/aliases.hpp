/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#pragma once

#include <cstdint>

// ---------------------------------------------------------------------------------------- aliases

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

namespace tengine {

class IAsset;
class IAssetLoader;
class Node;

// sets restriction for T - has to be final and has to inherit from IAsset
template <typename T>
concept IsAssetType = std::is_base_of_v<IAsset, T> && std::is_final_v<T>;

// declares ITypedAssetLoader with above restriction
template <IsAssetType T>
class ITypedAssetLoader;

using IAssetPtr = std::shared_ptr<IAsset>;
using IAssetLoaderPtr = std::unique_ptr<IAssetLoader>;

using NodePtr = std::shared_ptr<Node>;
using NodeHandle = std::weak_ptr<Node>;

class AssetManager;
using AssetManagerPtr = AssetManager*;

}  // namespace tengine
