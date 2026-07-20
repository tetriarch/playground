/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/scene_tree.hpp"

#include "tengine/ns/node.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

SceneTree::SceneTree() : root_(nullptr), updateState_(UpdateState::Idle) {
}

void SceneTree::setSceneRoot(const NodePtr& scene, UpdateType updateType) {
    if(updateState_ == UpdateState::Updating && updateType == UpdateType::Deferred) {
        modifications_.emplace_back([scene, this]() {
            setSceneRoot(scene, UpdateType::Immediate);
        });
        return;
    }

    // TODO: move this to immediate method
    TENGINE_ASSERT(scene, "scene is nullptr");
    TENGINE_ASSERT(!scene->tree_, "scene root already belongs to a tree");
    TENGINE_ASSERT(!scene->parent(), "scene root already has a parent");

    if(root_) {
        root_->setTree(nullptr);
    }

    root_ = scene;
    root_->setTree(this);
}

void SceneTree::addChild(const NodePtr& parent, const NodePtr& child, UpdateType updateType) {
    if(updateState_ == UpdateState::Updating && updateType == UpdateType::Deferred) {
        modifications_.emplace_back([parent, child, this]() {
            addChild(parent, child, UpdateType::Immediate);
        });
        return;
    }

    // TODO: move this to immediate method
    TENGINE_ASSERT(!child->parent(), "child node {} already has a parent", child->name());

    auto [_, inserted] = parent->children_.emplace(child->name_, child);
    TENGINE_ASSERT(
        inserted, "parent {} already has a child named {}", parent->name(), child->name()
    );

    child->setParent(parent);
    child->setTree(this);
    child->ready();
}

void SceneTree::removeChild(const NodePtr& parent, const NodePtr& child, UpdateType updateType) {
    if(updateState_ == UpdateState::Updating && updateType == UpdateType::Deferred) {
        modifications_.emplace_back([parent, child, this]() {
            removeChild(parent, child, UpdateType::Immediate);
        });
        return;
    }

    // TODO: move this to immediate method
    TENGINE_ASSERT(child->parent(), "child node {} doesn't have a parent", child->name());

    parent->children_.erase(child->name_);
    child->resetParent();
    child->setTree(nullptr);
}

void SceneTree::beginModificationQueue() {
    TENGINE_ASSERT(updateState_ == UpdateState::Idle, "Tree state == Updating");
    updateState_ = UpdateState::Updating;
}

void SceneTree::endModificationQueue() {
    TENGINE_ASSERT(updateState_ == UpdateState::Updating, "Tree state == Idle");
    updateState_ = UpdateState::Idle;
}

void SceneTree::applyModifications() {
    TENGINE_ASSERT(updateState_ == UpdateState::Idle, "Tree state == Updating");
    for(auto&& m : modifications_) {
        m();
    }
    modifications_.clear();
}

void SceneTree::ready() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->ready();
}

void SceneTree::update(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->update(dt);
}

void SceneTree::postUpdate(f32 dt) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->postUpdate(dt);
}

void SceneTree::render() {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    root_->render();
}

}  // namespace tengine
