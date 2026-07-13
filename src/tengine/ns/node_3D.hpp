/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/node.hpp"
#include "tengine/transform.hpp"

namespace tengine {

#pragma once

class Node3D : public Node {
public:
    Node3D();
    Node3D(const std::string& name);

public:
    void ready() override;
    void update(const f32 dt) override;
    void postUpdate(const f32 dt) override;
    void render() override;

public:
    [[nodiscard]] auto transform() const -> const Transform&;
    void setTransform(const Transform& transform);

private:
    Transform transform_;
};

}  // namespace tengine
