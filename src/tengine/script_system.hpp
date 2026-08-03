/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace tengine {

struct ScriptTable {
    std::string path;
    sol::table table;
};

using ScriptInstance = std::optional<ScriptTable>;
using ScriptFunction = std::optional<sol::safe_function>;
using ScriptResult = sol::protected_function_result;

struct ValidationResult {
    bool success = false;
    std::string error;

    explicit operator bool() const noexcept {
        return success;
    }
};

class ScriptSystem {
    struct InternalOnly {};

public:
    ScriptSystem(const InternalOnly&);
    static auto get() -> ScriptSystem*;

public:
    /**
     * @brief executes a Lua script from file and returns the table it produces
     *
     * @param filePath path to the script asset
     * @return valid ScriptInstance on success, std::nullopt on failure
     */
    [[nodiscard]] auto runFile(const std::string& filePath) -> ScriptInstance;

    /**
     * @brief executes a Lua script
     *
     * @param script Lua source code
     * @return true on success, false on failure
     */
    bool runScript(const std::string& script);

    /**
     * @brief validates execution result of Lua execution.
     *
     * @param executionResult result returned by Lua script or callback.
     * @return ValidationResult describing whether execution succeeded and error message on failure,
     */
    [[nodiscard]] auto validateExecution(const ScriptResult& executionResult) const
        -> ValidationResult;

    /**
     * @brief looks up function in a ScriptInstance
     *
     * @param name name of the function
     * @param instance script instance to search
     * @return valid ScriptFunction on success, std::nullopt on failure if the function does not
     * exist or is not callable
     */
    auto function(const std::string& name, const ScriptInstance& instance) const -> ScriptFunction;

private:
    /**
     * @brief Initializes the script system
     *
     * @return true on success, false on failure
     */
    [[nodiscard]] bool init();

    /**
     * @brief internal validation helper used during initialization and by validateExecution
     */
    [[nodiscard]] auto validateExecutionInternal(const ScriptResult& executionResult) const
        -> ValidationResult;

    void registerGlobalTypes();

private:
    sol::state state_;
    bool initialized_;
};

}  // namespace tengine
