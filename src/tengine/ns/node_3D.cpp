/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/node_3D.hpp"

namespace tengine {

Node3D::Node3D() : Node("Node3D") {
}

Node3D::Node3D(const std::string& name) : Node(name) {
}

auto Node3D::transform() const -> const Transform& {
    return transform_;
}

void Node3D::setTransform(const Transform& transform) {
    transform_ = transform;
}

}  // namespace tengine
