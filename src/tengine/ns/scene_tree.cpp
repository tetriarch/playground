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
        root_->unsetTree();
    } else {
        root_ = scene;
        root_->setTree(this);
    }
}

void SceneTree::registerForUpdate(const NodePtr& node) {
    TENGINE_ASSERT(node, "node is nullptr");

    // TODO: - check whether the node is already in the std::vector
    // - see std::ranges for that - vector holds weak references while we work with strong one here
    // - mark the tree as unordered as update has to happen in descending order from root to leaves
    updateNodes_.push_back(node);
    unordered_ = true;
}

void SceneTree::unregisterForUpdate(const NodePtr& node) {
    TENGINE_ASSERT(node, "node is nullptr");
    // TODO: - check whether the node is present in the std::vector
    // - see std::ranges for that - vector holds weak references while we work with strong one here
    // - reset resulting iterator if node is found
    // - mark the tree as unordered as update has to happen in descending order from root to leaves
    // - reseted node should be deleted

    // *it->reset();
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
    while(!updateNodes_.empty() && !updateNodes_.back().lock()) {
        updateNodes_.pop_back();
    }

    while(!renderNodes_.empty() && !renderNodes_.back().lock()) {
        renderNodes_.pop_back();
    }

    unordered_ = false;
}

void SceneTree::update(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");

    u64 count = updateNodes_.size();
    for(u64 i = 0; i < count; i++) {
        auto node = updateNodes_[i].lock();
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
