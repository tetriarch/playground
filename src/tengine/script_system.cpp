/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/script_system.hpp"

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

auto ScriptSystem::runFile(const std::string& filePath) -> sol::table {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");

    auto result = state_.safe_script_file(filePath);
    if(result.status() != sol::call_status::ok) {
        sol::error error = result;
        LOG_ERROR("ScriptSystem", "{} | {}", error.what(), filePath);
        return sol::table();
    }

    if(result.get_type() != sol::type::table) {
        LOG_ERROR("ScriptSystem", "{} | script does not return table", filePath);
        return sol::table();
    }
    return sol::table(result);
}

bool ScriptSystem::runScript(const std::string& script) {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");
    return runScriptInternal(script);
}

bool ScriptSystem::runScriptInternal(const std::string& script) {
    auto result = state_.safe_script(script);
    if(result.status() != sol::call_status::ok) {
        sol::error error = result;
        LOG_ERROR("ScriptSystem", "{}", error.what());
        return false;
    }
    return true;
}

}  // namespace tengine
