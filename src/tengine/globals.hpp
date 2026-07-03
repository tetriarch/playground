/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#pragma once

#include <tengine/aliases.hpp>

namespace tengine {

class Logger;

[[nodiscard]] auto logger() -> const std::shared_ptr<Logger>&;
void setLogger(const std::shared_ptr<Logger>& logger);

// TODO: this should not be exposed
// registers fallback assets / requires valid GPU context to be set
void registerFallbackAssets(AssetManagerPtr assetManager);

}  // namespace tengine
