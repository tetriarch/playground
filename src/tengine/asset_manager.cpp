/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include "tengine/asset_manager.hpp"

#include "tengine/utils/file_io.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

static std::shared_ptr<AssetManager> s_assetManager_;

AssetManager* AssetManager::get() {
    if(!s_assetManager_) {
        s_assetManager_ = std::make_shared<AssetManager>(InternalOnly{});
    }
    return s_assetManager_.get();
}

void AssetManager::shutdown() {
    s_assetManager_ = nullptr;
}

void AssetManager::setAssetRoot(const std::string& rootPath) {
    assetRoot_ = rootPath;
}

auto AssetManager::getAssetPath(const std::string& assetPath) const -> std::filesystem::path {
    return std::filesystem::path(assetRoot_) / assetPath;
}

bool AssetManager::isValidAssetPath(std::string const& assetPath) const {
    auto path = std::filesystem::path(assetRoot_) / assetPath;
    return std::filesystem::is_regular_file(path);
}

void AssetManager::registerLoader(std::type_index type, IAssetLoaderPtr loader) {
    if(auto it = assetLoaders_.find(type); it != assetLoaders_.end()) {
        LOG_DEBUG("AssetManager", "loader {} is already registered - skipping", type.name());
        return;
    }
    assetLoaders_.emplace(type, std::move(loader));
}

auto AssetManager::load(std::type_index assetType, const std::string& assetPath) -> IAssetPtr {
    // if already loaded return it
    if(auto it = assets_.find(assetPath); it != assets_.end()) {
        auto ptr = it->second.lock();
        if(ptr) {
            return ptr;
        }

        assets_.erase(assetPath);
    }

    // find suitable loader
    auto loaderIt = assetLoaders_.find(assetType);
    if(loaderIt == assetLoaders_.end()) {
#ifdef DEBUG
        LOG_ERROR("AssetManager", "failed to find loader for {}", assetType.name());
        return nullptr;
#else
        LOG_FATAL("AssetManager", "failed to find loader for {}", assetType.name());
#endif
    }

    const auto& loader = loaderIt->second;

    // validate assetPath
    auto path = getAssetPath(assetPath);
    if(!std::filesystem::exists(path)) {
#ifdef DEBUG
        LOG_ERROR("AssetManager", "asset at {} does not exist", path.generic_string());
        return nullptr;
#else
        LOG_FATAL("AssetManager", "asset at {} does not exist", path.generic_string());
#endif
    }

    // in case something in the loader failed
    auto asset = loader->load(*this, path.generic_string());
    if(!asset) {
#ifdef DEBUG
        LOG_ERROR("AssetManager", "asset at {} failed to load", path.generic_string());
        return nullptr;
#else
        LOG_FATAL("AssetManager", "asset at {} failed to load", path.generic_string());
#endif
    }

    LOG_DEBUG("AssetManager", "asset {} loaded successfuly", path.generic_string());
    assets_.emplace(assetPath, asset);

    return asset;
}

auto AssetManager::loadRaw(std::string const& assetPath) -> std::optional<std::vector<u8>> {
    auto path = getAssetPath(assetPath);
    if(!std::filesystem::exists(path)) {
        LOG_ERROR("AssetManager", "asset at {} does not exist", path.generic_string());
        return std::nullopt;
    }

    auto result = file_io::readBinaryFile(path.generic_string());
    if(!result.has_value()) {
        LOG_ERROR("AssetManager", "asset at {} failed to load", path.generic_string());
        return std::nullopt;
    }

    return result.value();
}

void AssetManager::registerAsset(const std::string& assetName, IAssetPtr asset) {
    assets_.emplace(assetName, asset);
}

void AssetManager::clear() {
    assets_.clear();
    assetLoaders_.clear();
}

}  // namespace tengine
