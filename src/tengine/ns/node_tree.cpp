/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/node_tree.hpp"

#include "tengine/ns/node.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

NodeTree::NodeTree() : root_(std::make_shared<Node>("~")) {
}

void NodeTree::ready() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->ready();
}

void NodeTree::update(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->update(dt);
}

void NodeTree::postUpdate(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->postUpdate(dt);
}

void NodeTree::render() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->render();
}

}  // namespace tengine
