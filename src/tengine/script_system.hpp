/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace tengine {

using ScriptInstance = std::optional<sol::table>;

class ScriptSystem {
    struct InternalOnly {};

public:
    ScriptSystem(const InternalOnly&);
    static auto get() -> ScriptSystem*;

public:
    /**
     * @brief runs a Lua script from file and returns it's content as Lua table.
     *
     * @param filePath
     * @return valid ScriptInstance on success or std::nullopt on failure
     */
    auto runFile(const std::string& filePath) -> ScriptInstance;

    /**
     * @brief runs a Lua script directly
     *
     * @param script
     * @return true on success, false on failure
     */
    bool runScript(const std::string& script);

private:
    /**
     * @brief Initializes the script system
     *
     * @return true on success, false on failure
     */
    [[nodiscard]] bool init();

    /**
     * @brief internal delegate of init, runFile and runScript
     *
     * @param script
     * @param filePath - filePath of the script, leave empty, if you dont run script from a file
     * from a file
     * @return valid sol::protected_function_result on success or std::nullopt on failure
     */
    auto runScriptInternal(const std::string& script, const std::string& filePath = "")
        -> std::optional<sol::protected_function_result>;

private:
    sol::state state_;
    bool initialized_;
};

}  // namespace tengine
