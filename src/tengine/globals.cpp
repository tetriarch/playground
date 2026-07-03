/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include "tengine/utils/logger.hpp"

namespace tengine {

namespace {

std::shared_ptr<Logger> g_logger;

}  // namespace

[[nodiscard]] auto logger() -> const std::shared_ptr<Logger>& {
    assert(g_logger && "logger is not initialized");
    return g_logger;
}

void setLogger(const std::shared_ptr<Logger>& logger) {
    assert(logger && "initializing logger with nullptr");
    assert(!g_logger && "logger is already initialized");
    g_logger = logger;
}

}  // namespace tengine
