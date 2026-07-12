/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/aliases.hpp"

#pragma once

namespace tengine {

class NodeTree {
public:
    NodeTree();

public:
    void ready();
    void update(f32 dt);
    void postUpdate(f32 dt);
    void render();

public:
    auto root() const -> const Node*;

private:
    NodePtr root_;
};

}  // namespace tengine
