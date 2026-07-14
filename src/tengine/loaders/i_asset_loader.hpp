/*
* ------
* Copyright (c) 2026 Petr Jirmus
* All rights reserved.
*/

#pragma once

#include "tengine/aliases.hpp"
#include "tengine/globals.hpp"

namespace tengine {

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    virtual auto load(AssetManager& assetManager, const std::string& assetPath) -> IAssetPtr = 0;

private:
    template <IsAssetType T>
    friend class ITypedAssetLoader;

    // forbids inheritance by non friend types
    // always derive from ITypedAssetLoader
    IAssetLoader() = default;
};

template <IsAssetType T>
class ITypedAssetLoader : public IAssetLoader {};

}  // namespace tengine
