/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#pragma once

#include <cstdarg>
#include <lua.hpp>
#include <tengine/aliases.hpp>
#include <tengine/utils/logger.hpp>

namespace tengine {

class IFnBinding;

template <class T>
class FnBinding;

template <class T>
struct Fn : Fn<decltype(&T::operator())> {};

// free function support
template <class Ret, class... FnArgs>
struct Fn<Ret (*)(FnArgs...)> {
    using return_type = Ret;

    // unfold FnArgs into a tuple
    using args_tuple = std::tuple<FnArgs...>;

    // argument count
    static constexpr u64 argc = sizeof...(FnArgs);

    // easier arg access
    template <u64 I>
    using arg = std::tuple_element_t<I, args_tuple>;

    // compile time list for fold expression
    static constexpr auto argIndices = std::make_index_sequence<argc>{};
};

// lambda support
template <class T, class Ret, class... FnArgs>
struct Fn<Ret (T::*)(FnArgs...) const> {
    using return_type = Ret;

    // unfold FnArgs into a tuple
    using args_tuple = std::tuple<FnArgs...>;

    // argument count
    static constexpr u64 argc = sizeof...(FnArgs);

    // easier arg access
    template <u64 I>
    using arg = std::tuple_element_t<I, args_tuple>;

    // compile time list for fold expression
    static constexpr auto argIndices = std::make_index_sequence<argc>{};
};

// to ensure T has function traits
template <class T>
concept CallableFromLua = requires {
    typename Fn<T>::return_type;
    Fn<T>::argc;
};

namespace detail {

s32 luaFnHook(lua_State* L);

};  // namespace detail

/**
 * @class LuaRuntime
 * @brief Holds Lua state and provides tools to execute Lua scripts, bind functions and read data
 * from scripts
 */
class LuaRuntime {
public:
    LuaRuntime();
    ~LuaRuntime();
    LuaRuntime(const LuaRuntime&) = delete;
    LuaRuntime& operator=(const LuaRuntime&) = delete;

public:
    /**
     * @brief Opens Lua libraries
     */
    void openLibs() const;

    /**
     * @brief runs Lua script from file
     *
     * @param scriptPath
     */
    void runFile(const std::string& scriptPath) const;

    /**
     * @brief runs Lua script directly from string
     *
     * @param string
     */
    void runString(const std::string& scriptString) const;

    /**
     * @brief Binds static function to be used in Lua
     * the format of the function has to be int fn(lua_State* L)
     *
     * @param functionName name by which is the function going to be called in Lua
     * @param function function to bind
     */
    void bindRaw(const std::string& functionName, lua_CFunction function) const;

    /**
     * @brief Binds any Callable to Lua
     *
     * @param functionName
     * @param function
     */
    template <CallableFromLua T>
    void bind(const std::string& functionName, T function) {
        auto [it, added] =
            bindings_.emplace(functionName, std::make_unique<FnBinding<T>>(std::move(function)));
        if(!added) {
            LOG_ERROR("LuaRuntime", "attempting to bind function with same name");
            return;
        }

        auto binding = it->second.get();
        lua_pushlightuserdata(L_, binding);
        lua_pushcclosure(L_, detail::luaFnHook, 1);
        lua_setglobal(L_, functionName.c_str());
    }

    /**
     * @brief Calls Lua function from script
     *
     * @param funcitonName
     * @param args arguments to pass down to the Lua function
     */
    template <class... TArgs>
    void call(const std::string& functionName, const TArgs&... args) const {
        lua_getglobal(L_, functionName.c_str());
        if(!lua_isfunction(L_, -1)) {
            LOG_FATAL("LuaRuntime", "'{}' is not a function", functionName);
        }

        (push(L_, args), ...);

        if(lua_pcall(L_, sizeof...(args), 0, 0)) {
            LOG_FATAL("LuaRuntime", "{}", lua_tostring(L_, -1));
        }
    }

    /**
     * @brief Calls Lua function from script with return value
     *
     * @tparam TArgs
     * @param funcitonName
     * @param args arguments to pass down to the Lua function
     * @return value of type T
     */
    template <class Ret, class... TArgs>
        requires(!std::is_void_v<Ret>)
    Ret callRet(const std::string& functionName, const TArgs&... args) const {
        lua_getglobal(L_, functionName.c_str());
        if(!lua_isfunction(L_, -1)) {
            LOG_FATAL("LuaRuntime", "'{}' is not a function", functionName);
        }

        (push(L_, args), ...);

        if(lua_pcall(L_, sizeof...(args), 1, 0)) {
            LOG_FATAL("LuaRuntime", "{}", lua_tostring(L_, -1));
        }

        Ret v = pull<Ret>(L_, -1);
        lua_pop(L_, 1);
        return v;
    }

    /**
     * @brief Pushes values to Lua stack to be used by Lua script
     *
     * @tparam T can be int, float, bool, const char*
     * @param value
     */
    template <class T>
    static void push(lua_State* L, const T value);

    /**
     * @brief Pulles value from Lua stack. Poping value from the stack is responsibility of caller
     * of this function.
     *
     * @tparam T return type
     * @return T value
     */
    template <class T>
    static T pull(lua_State* L, s32 index);

private:
    lua_State* L_;
    std::unordered_map<std::string, std::unique_ptr<IFnBinding>> bindings_;
};

template <>
inline void LuaRuntime::push(lua_State* L, s32 value) {
    lua_pushinteger(L, value);
}

template <>
inline void LuaRuntime::push(lua_State* L, f32 value) {
    lua_pushnumber(L, value);
}

template <>
inline void LuaRuntime::push(lua_State* L, bool value) {
    lua_pushboolean(L, value);
}

template <>
inline void LuaRuntime::push(lua_State* L, const char* value) {
    lua_pushstring(L, value);
}

template <>
inline void LuaRuntime::push(lua_State* L, const std::string& value) {
    lua_pushstring(L, value.c_str());
}

template <>
inline s32 LuaRuntime::pull(lua_State* L, s32 index) {
    if(!lua_isinteger(L, -1)) {
        LOG_FATAL("LuaRuntime", "expected integer value, got {}", luaL_typename(L, index));
    }
    return static_cast<s32>(lua_tointeger(L, index));
}

template <>
inline f32 LuaRuntime::pull(lua_State* L, s32 index) {
    if(!lua_isnumber(L, -1)) {
        LOG_FATAL("LuaRuntime", "expected number value, got {}", luaL_typename(L, index));
    }
    return static_cast<f32>(lua_tonumber(L, index));
}

template <>
inline bool LuaRuntime::pull(lua_State* L, s32 index) {
    if(!lua_isboolean(L, -1)) {
        LOG_FATAL("LuaRuntime", "expected bool value, got {}", luaL_typename(L, index));
    }
    return static_cast<bool>(lua_toboolean(L, index));
}

template <>
inline std::string LuaRuntime::pull(lua_State* L, s32 index) {
    if(lua_type(L, -1) != LUA_TSTRING) {
        LOG_FATAL("LuaRuntime", "expected string value, got {}", luaL_typename(L, index));
    }
    return static_cast<std::string>(lua_tostring(L, index));
}

// type "erasure" of bound function
class IFnBinding {
public:
    virtual ~IFnBinding() = default;
    virtual int invoke(lua_State* L) = 0;
};

template <class T>
class FnBinding : public IFnBinding {
public:
    explicit FnBinding(T fn) : fn_(std::move(fn)) {
    }

public:
    s32 invoke(lua_State* L) override {
        using Traits = Fn<T>;
        return invokeImpl<Traits>(L, Traits::argIndices);
    }

private:
    /**
     * @brief invoke internal to utilize compile time indices
     *
     * @tparam Traits
     * @param L
     * @return
     */
    template <class Traits, u64... I>
    s32 invokeImpl(lua_State* L, std::index_sequence<I...>) {
        using Ret = typename Traits::return_type;

        if constexpr(std::is_void_v<Ret>) {
            fn_(LuaRuntime::pull<typename Traits::template arg<I>>(L, I + 1)...);
            return 0;
        } else {
            // I + 1 because Lua indexing starts at 1 not 0
            Ret result = fn_(LuaRuntime::pull<typename Traits::template arg<I>>(L, I + 1)...);
            LuaRuntime::push(L, result);
            return 1;
        }
    }

private:
    T fn_;
};

}  // namespace tengine
