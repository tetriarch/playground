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
    [[nodiscard]] auto transform() const -> const Transform&;
    void setTransform(const Transform& transform);

private:
    Transform transform_;
};

}  // namespace tengine
