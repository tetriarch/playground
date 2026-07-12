/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/node.hpp"

namespace tengine {

#pragma once

class Node3D : public Node {
public:
    Node3D();

public:
    void ready() override;
    void update(const f32 dt) override;
    void postUpdate(const f32 dt) override;
    void render() override;

private:
    // TODO: Transform
};

}  // namespace tengine
