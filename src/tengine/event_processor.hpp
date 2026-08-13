/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#pragma once

#include <SDL3/SDL_events.h>

// -------------------------------------------------------------------------------------------------
//
// THIS IS A TEMPORARY SYSTEM, NOT A FINAL SOLUTION !!
//
// -------------------------------------------------------------------------------------------------

namespace tengine {

class EventProcessor {
    struct InternalOnly {};

public:
    static auto get() -> EventProcessor*;
    EventProcessor(const InternalOnly&);

public:
    void handleEvent(const SDL_Event& event);

public:
    void map(SDL_Scancode key, const std::string& eventType);

private:
    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);

private:
    std::unordered_map<SDL_Scancode, std::string> keyToEvent_;
};

}  // namespace tengine
