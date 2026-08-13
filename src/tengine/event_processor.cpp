/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/event_processor.hpp"

#include "tengine/utils/logger.hpp"

// -------------------------------------------------------------------------------------------------
//
// THIS IS A TEMPORARY SYSTEM, NOT A FINAL SOLUTION !!
//
// -------------------------------------------------------------------------------------------------

namespace tengine {
namespace {

std::unique_ptr<EventProcessor> s_inputHandler;

}

auto EventProcessor::get() -> EventProcessor* {
    if(!s_inputHandler) {
        s_inputHandler = std::make_unique<EventProcessor>(InternalOnly{});
    }
    return s_inputHandler.get();
}

EventProcessor::EventProcessor(const InternalOnly&) {
}

void EventProcessor::handleEvent(const SDL_Event& event) {
    switch(event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            handleKeyboardEvent(event);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_WHEEL:
            handleMouseEvent(event);
            break;
        default:
            break;
    }
}

void EventProcessor::map(SDL_Scancode key, const std::string& eventType) {
    keyToEvent_[key] = eventType;
}

void EventProcessor::handleKeyboardEvent(const SDL_Event& event) {
}

void EventProcessor::handleMouseEvent(const SDL_Event& event) {
}

}  // namespace tengine
