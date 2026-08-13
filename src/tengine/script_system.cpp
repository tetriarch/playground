/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/script_system.hpp"

#include "tengine/aliases.hpp"
#include "tengine/asset_manager.hpp"
#include "tengine/transform.hpp"
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
    auto result = state_.safe_script(
        "package.path = package.path .. \";assets/scripts/?.lua;assets/tengine/scripts/?.lua\""
    );
    auto validation = validateExecutionInternal(result);
    if(!validation) {
        LOG_ERROR("ScriptSystem", "{}", validation.error);
        initialized_ = false;
        return initialized_;
    }
    registerGlobalTypes();
    initialized_ = true;
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
    auto result = state_.safe_script(scriptStr, sol::script_pass_on_error);

    auto validation = validateExecution(result);
    if(!validation) {
        LOG_ERROR(
            "ScriptSystem", "{} | {}", validation.error, am->getAssetPath(filePath).generic_string()
        );
        return std::nullopt;
    }

    if(result.get_type() != sol::type::table) {
        LOG_ERROR(
            "ScriptSystem", "script does not return table | {}",
            am->getAssetPath(filePath).generic_string()
        );
        return std::nullopt;
    }

    ScriptTable table{filePath, static_cast<sol::table>(result)};
    return table;
}

bool ScriptSystem::runScript(const std::string& script) {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");

    auto result = state_.safe_script(script, sol::script_pass_on_error);
    auto validation = validateExecution(result);
    if(!validation) {
        LOG_ERROR("ScriptSystem", "{}", validation.error);
        return false;
    }
    return true;
}

auto ScriptSystem::validateExecution(const ScriptResult& executionResult) const
    -> ValidationResult {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");
    return validateExecutionInternal(executionResult);
}

auto ScriptSystem::function(const std::string& name, const ScriptInstance& instance) const
    -> ScriptFunction {
    TENGINE_ASSERT(initialized_, "ScriptSystem is not initialized");
    TENGINE_ASSERT(instance, "instance is null");

    auto instVal = instance.value();
    auto fn = instVal.table[name];
    if(!fn.is<sol::safe_function>()) {
        LOG_ERROR("ScriptSystem", "script {} has {} but it is not a function", instVal.path, name);
        return std::nullopt;
    }
    return fn;
}

[[nodiscard]] auto ScriptSystem::validateExecutionInternal(
    const ScriptResult& executionResult
) const -> ValidationResult {
    if(!executionResult.valid()) {
        sol::error err = executionResult;
        std::string error = err.what();
        return {false, std::move(error)};
    }
    return {true, {}};
}

void ScriptSystem::registerGlobalTypes() {
    auto vec3Type =
        state_.new_usertype<glm::vec3>("Vec3", sol::constructors<glm::vec3(f32, f32, f32)>());
    vec3Type["x"] = &glm::vec3::x;
    vec3Type["y"] = &glm::vec3::y;
    vec3Type["z"] = &glm::vec3::z;

    auto quatType = state_.new_usertype<glm::quat>("Quat");
    quatType["x"] = &glm::quat::x;
    quatType["y"] = &glm::quat::y;
    quatType["z"] = &glm::quat::z;
    quatType["w"] = &glm::quat::w;

    auto transformType = state_.new_usertype<Transform>("Transform");
    transformType["position"] = &Transform::position;
    transformType["rotation"] = &Transform::rotation;
    transformType["scale"] = &Transform::scale;
}

}  // namespace tengine
