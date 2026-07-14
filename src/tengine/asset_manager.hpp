/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#pragma once

#include "tengine/aliases.hpp"
#include "tengine/loaders/i_asset_loader.hpp"

namespace tengine {

class AssetManager {
    struct InternalOnly {};

public:
    static AssetManager* get();
    static void shutdown();

    AssetManager(InternalOnly const&) {
    }

public:
    void setAssetRoot(const std::string& rootPath);

    auto getAssetPath(const std::string& assetPath) const -> std::filesystem::path;

    bool isValidAssetPath(std::string const& assetPath) const;

    void registerLoader(std::type_index type, IAssetLoaderPtr loader);

    template <IsAssetType TAsset, typename TAssetLoader>
    void registerLoader(std::unique_ptr<TAssetLoader> loader)
        requires(std::is_base_of_v<ITypedAssetLoader<TAsset>, TAssetLoader>);

    auto load(std::type_index assetType, const std::string& assetpath) -> IAssetPtr;

    auto loadRaw(std::string const& assetPath) -> std::optional<std::vector<u8>>;

    template <IsAssetType TAsset>
    auto load(const std::string& assetPath) -> std::shared_ptr<TAsset>;

    void registerAsset(const std::string& assetName, IAssetPtr asset);

    void clear();

private:
    std::string assetRoot_;
    std::unordered_map<std::string, std::weak_ptr<IAsset>> assets_;
    std::unordered_map<std::type_index, IAssetLoaderPtr> assetLoaders_;
};

template <IsAssetType TAsset, typename TAssetLoader>
inline void AssetManager::registerLoader(std::unique_ptr<TAssetLoader> loader)
    requires(std::is_base_of_v<ITypedAssetLoader<TAsset>, TAssetLoader>)
{
    registerLoader(typeid(TAsset), std::move(loader));
}

template <IsAssetType TAsset>
inline auto AssetManager::load(const std::string& assetPath) -> std::shared_ptr<TAsset> {
    auto assetPtr = load(typeid(TAsset), assetPath);
    return std::static_pointer_cast<TAsset>(std::move(assetPtr));
}

}  // namespace tengine
