/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/scene_tree.hpp"

#include "tengine/ns/node.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

SceneTree::SceneTree() : active_(false), root_(nullptr), state_(TreeState::Idle) {
}

void SceneTree::setSceneRoot(const NodePtr& scene) {
    TENGINE_ASSERT(
        state_ != TreeState::GettingReady,
        "setting scene root while tree is getting ready is not allowed"
    );
    if(state_ == TreeState::Updating) {
        modifications_.emplace_back([scene, this]() { setSceneRootImmediate(scene); });
        return;
    }
    setSceneRootImmediate(scene);
}

void SceneTree::addChild(const NodePtr& parent, const NodePtr& child) {
    if(state_ == TreeState::Updating || state_ == TreeState::GettingReady) {
        modifications_.emplace_back([parent, child, this]() { addChildImmediate(parent, child); });
        return;
    }
    addChildImmediate(parent, child);
}

void SceneTree::removeChild(const NodePtr& parent, const NodePtr& child) {
    if(state_ == TreeState::Updating || state_ == TreeState::GettingReady) {
        modifications_.emplace_back([parent, child, this]() {
            removeChildImmediate(parent, child);
        });
        return;
    }
    removeChildImmediate(parent, child);
}

void SceneTree::beginModificationQueue() {
    TENGINE_ASSERT(state_ == TreeState::Idle, "tree is not idle");
    state_ = TreeState::Updating;
}

void SceneTree::endModificationQueue() {
    TENGINE_ASSERT(state_ == TreeState::Updating, "tree is not updating");
    state_ = TreeState::Idle;
}

void SceneTree::applyModifications() {
    TENGINE_ASSERT(state_ == TreeState::Idle, "tree is not idle");

    // we need to iterate modifications in waves. In case there is need for deferred handling when
    // iterating modifications

    while(!modifications_.empty()) {
        auto currentMods = std::move(modifications_);
        modifications_.clear();

        for(auto&& cm : currentMods) {
            cm();
        }
    }
}

void SceneTree::ready() {
    TENGINE_ASSERT(
        state_ != TreeState::GettingReady, "ready cannot be called while the tree is getting ready"
    );

    if(state_ == TreeState::Updating) {
        modifications_.emplace_back([this]() { readyImmediate(); });
        return;
    }
    readyImmediate();
    // we apply modifications in case that any node affected by ready call created new modification
    // to the tree
    applyModifications();
}

void SceneTree::update(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    TENGINE_ASSERT(active_, "tree is not ready");
    root_->update(dt);
}

void SceneTree::postUpdate(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    TENGINE_ASSERT(active_, "tree is not ready");
    root_->postUpdate(dt);
}

void SceneTree::render() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    TENGINE_ASSERT(active_, "tree is not ready");
    root_->render();
}

auto SceneTree::root() const -> const Node* {
    return root_.get();
}

void SceneTree::readyImmediate() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    TENGINE_ASSERT(!active_, "tree is already ready");

    active_ = true;
    state_ = TreeState::GettingReady;
    root_->ready();
    state_ = TreeState::Idle;
}

void SceneTree::setSceneRootImmediate(const NodePtr& scene) {
    TENGINE_ASSERT(state_ == TreeState::Idle, "tree is not idle");
    TENGINE_ASSERT(scene, "scene is nullptr");
    TENGINE_ASSERT(!scene->tree_, "scene root already belongs to a tree");
    TENGINE_ASSERT(!scene->parent(), "scene root already has a parent");

    // detach/destroy old scene
    if(root_) {
        root_->setTree(nullptr);
    }

    active_ = false;
    root_ = scene;
    root_->setTree(this);
}

void SceneTree::addChildImmediate(const NodePtr& parent, const NodePtr& child) {
    TENGINE_ASSERT(state_ == TreeState::Idle, "tree is not idle");
    TENGINE_ASSERT(parent, "parent is nullptr");
    TENGINE_ASSERT(parent->tree_ == this, "parent is not from this tree");
    TENGINE_ASSERT(child, "child is nullptr");
    TENGINE_ASSERT(!child->tree_, "child {} already belongs to a tree", child->name());
    TENGINE_ASSERT(!child->parent(), "child {} already has a parent", child->name());
    TENGINE_ASSERT(child.get() != parent.get(), "node {} cannot be it's own child", child->name());
    TENGINE_ASSERT(
        !child->isAncestorOf(parent), "adding {} under {} would create hierarchy cycle",
        child->name(), parent->name()
    );

    auto [_, inserted] = parent->children_.emplace(child->name_, child);
    // TODO: we should check inserted and if it is false
    // change the child's name with iterative number before trying to add it, instead of asserting
    // here (example: if parent has Child named Something it should become Something002, or similar,
    // it might be worth to enforce this with RegEx check
    TENGINE_ASSERT(
        inserted, "parent {} already has a child named {}", parent->name(), child->name()
    );

    child->setParent(parent);
    child->setTree(this);

    // ready the child when entering the tree if the tree is active otherwise we ready the child by
    // calling ready of the entire tree
    if(active_) {
        readyNodeImmediate(child);
    }
}

void SceneTree::removeChildImmediate(const NodePtr& parent, const NodePtr& child) {
    TENGINE_ASSERT(state_ == TreeState::Idle, "tree is not idle");
    TENGINE_ASSERT(parent, "parent is nullptr");
    TENGINE_ASSERT(child, "child is nullptr");
    TENGINE_ASSERT(parent->tree_ == this, "parent is not from this tree");
    TENGINE_ASSERT(child->tree_ == this, "child is not from this tree");

    // confirm relationship both ways
    auto actualParent = child->parent();
    TENGINE_ASSERT(actualParent, "child {} does not have a parent", child->name());
    TENGINE_ASSERT(
        actualParent.get() == parent.get(), "node {} is not a child of {}", child->name(),
        parent->name()
    );

    auto it = parent->children_.find(child->name());

    TENGINE_ASSERT(
        it != parent->children_.end() && it->second == child,
        "parent-child hierarchy is inconsistent"
    );

    parent->children_.erase(it);
    child->resetParent();
    child->setTree(nullptr);
}

void SceneTree::readyNodeImmediate(const NodePtr& node) {
    TENGINE_ASSERT(state_ == TreeState::Idle, "tree is not idle");
    TENGINE_ASSERT(node, "node is nullptr");
    TENGINE_ASSERT(node->tree_ == this, "child is not from this tree");

    state_ = TreeState::GettingReady;
    node->ready();
    state_ = TreeState::Idle;
}

}  // namespace tengine
