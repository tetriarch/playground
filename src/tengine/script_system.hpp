/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace tengine {

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
     * @return populated Lua table on success. invalid table on failure, validity check is required
     * before using
     */
    auto runFile(const std::string& filePath) -> sol::table;

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
     * @brief internal delegate of runScript
     *
     * @param script
     * @return true on success, false on failure
     */
    bool runScriptInternal(const std::string& script);

private:
    sol::state state_;
    bool initialized_;
};

}  // namespace tengine
