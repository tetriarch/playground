/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/script_system.hpp"

#include "tengine/asset_manager.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

static std::unique_ptr<ScriptSystem> s_scriptSystem;

ScriptSystem::ScriptSystem(const InternalOnly&) : initialized_(false) {
}

auto ScriptSystem::get() -> ScriptSystem* {
    if(!s_scriptSystem) {
        s_scriptSystem = std::make_unique<ScriptSystem>(InternalOnly{});
        if(!s_scriptSystem->init()) {
            LOG_FATAL("ScriptSystem", "failed to initialize");
        }
    }
    return s_scriptSystem.get();
};

bool ScriptSystem::init() {
    TENGINE_ASSERT(!initialized_, "ScriptSystem is already initialized");

    // enable print, assert, require
    state_.open_libraries(sol::lib::base, sol::lib::package);
    if(runScriptInternal("package.path = package.path .. \";assets/scripts/?.lua\"")) {
        initialized_ = true;
    }
    return initialized_;
}

auto ScriptSystem::runFile(const std::string& filePath) -> ScriptInstance {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");

    auto am = AssetManager::get();
    auto scriptFileData = am->loadRaw(filePath);
    if(!scriptFileData) {
        // error already logged in AssetManager
        return std::nullopt;
    }

    const auto& script = scriptFileData.value();
    std::string scriptStr(reinterpret_cast<const char*>(script.data()), script.size());
    auto result = runScriptInternal(scriptStr, am->getAssetPath(filePath).generic_string());
    if(!result) {
        // error already logged in runScriptInternal
        return std::nullopt;
    }

    if(result.value().get_type() != sol::type::table) {
        LOG_ERROR(
            "ScriptSystem", "script does not return table | {}",
            am->getAssetPath(filePath).generic_string()
        );
        return std::nullopt;
    }
    return sol::table(result.value());
}

bool ScriptSystem::runScript(const std::string& script) {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");
    if(!runScriptInternal(script)) {
        return false;
    }
    return true;
}

auto ScriptSystem::runScriptInternal(const std::string& script, const std::string& filePath)
    -> std::optional<sol::protected_function_result> {
    auto result = state_.safe_script(script);
    if(result.status() != sol::call_status::ok) {
        sol::error error = result;

        if(!filePath.empty()) {
            LOG_ERROR("ScriptSystem", "{} | {}", error.what(), filePath);
        } else {
            LOG_ERROR("ScriptSystem", "{}", error.what());
        }

        return std::nullopt;
    }
    return result;
}

}  // namespace tengine
