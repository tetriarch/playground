/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/scene_tree.hpp"

#include "tengine/ns/node.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

SceneTree::SceneTree() : root_(nullptr), unordered_(false) {
}

void SceneTree::setSceneRoot(const NodePtr& scene) {
    if(root_ || !scene) {
        root_->exitTree();
    } else {
        root_ = scene;
        root_->enterTree(this);
    }
}

void SceneTree::registerForUpdate(const NodePtr& node) {
    TENGINE_ASSERT(node, "node is nullptr");
    TENGINE_ASSERT(
        std::ranges::find(updateNodes_, node) == updateNodes_.end(),
        "node {} is already registered for update", node->name()
    );

    updateNodes_.push_back(node);
    unordered_ = true;
}

void SceneTree::unregisterForUpdate(const NodePtr& node) {
    TENGINE_ASSERT(node, "node is nullptr");

    auto it = std::ranges::find(updateNodes_, node);
    TENGINE_ASSERT(it != updateNodes_.end(), "node {} is not registered for update", node->name());

    *it = nullptr;
    unordered_ = true;
}

void SceneTree::registerForRender(const NodePtr& node) {
    TENGINE_ASSERT(node, "node is nullptr");
    TENGINE_ASSERT(
        std::ranges::find(renderNodes_, node) == renderNodes_.end(),
        "node {} is already registered for render", node->name()
    );

    renderNodes_.push_back(node);
    unordered_ = true;
}

void SceneTree::unregisterForRender(const NodePtr& node) {
    TENGINE_ASSERT(node, "node is nullptr");

    auto it = std::ranges::find(renderNodes_, node);
    TENGINE_ASSERT(it != renderNodes_.end(), "node {} is not registered for render", node->name());

    *it = nullptr;
    unordered_ = true;
}

void SceneTree::orderTreeUpdates() {
    if(!unordered_) {
        return;
    }

    // comparison function, lower depth == earlier update
    auto compare = [](const NodeHandle& lhs, const NodeHandle& rhs) {
        auto l = lhs.lock();
        auto r = rhs.lock();

        if(!l) return false;
        if(!r) return true;

        auto lDepth = l->depth();
        auto rDepth = r->depth();

        return lDepth < rDepth;
    };

    // sort lists
    std::sort(updateNodes_.begin(), updateNodes_.end(), compare);
    std::sort(renderNodes_.begin(), renderNodes_.end(), compare);

    // remove null nodes at the end of lists
    while(!updateNodes_.empty() && !updateNodes_.back()) {
        updateNodes_.pop_back();
    }

    while(!renderNodes_.empty() && !renderNodes_.back()) {
        renderNodes_.pop_back();
    }

    unordered_ = false;
}

void SceneTree::update(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");

    u64 count = updateNodes_.size();
    for(u64 i = 0; i < count; i++) {
        auto node = updateNodes_[i];
        if(node) {
            node->update(dt);
        }
    }
}

void SceneTree::render() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->render();
}

auto SceneTree::root() const -> const Node* {
    return root_.get();
}

}  // namespace tengine
