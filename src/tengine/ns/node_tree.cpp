/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/ns/node_tree.hpp"

#include "tengine/ns/node.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

NodeTree::NodeTree() : root_(std::make_shared<Node>("~")), updateState_(UpdateState::Idle) {
    root_->tree_ = this;
}

void NodeTree::setSceneRoot(const NodePtr& scene) {
    TENGINE_ASSERT(root_, "root_ is nullptr");
    TENGINE_ASSERT(root_->children_.size() <= 1, "root contains more than one child");

    auto& children = root_->children_;
    if(children.empty()) {
        root_->addChild(scene);
        return;
    }

    auto& sceneRoot = children.begin()->second;
    sceneRoot->addChild(scene);
}

void NodeTree::addChild(const NodePtr& parent, const NodePtr& child, UpdateType updateType) {
    if(updateState_ == UpdateState::Updating && updateType == UpdateType::Deferred) {
        modifications_.emplace_back([parent, child, this]() {
            addChild(parent, child, UpdateType::Immediate);
        });
        return;
    }
    TENGINE_ASSERT(!child->parent(), "child node {} already has a parent", child->name());
    child->parent_ = parent;
    child->tree_ = this;
    child->ready();
    parent->children_.emplace(child->name_, child);
}

void NodeTree::removeChild(const NodePtr& parent, const NodePtr& child, UpdateType updateType) {
    if(updateState_ == UpdateState::Updating && updateType == UpdateType::Deferred) {
        modifications_.emplace_back([parent, child, this]() {
            removeChild(parent, child, UpdateType::Immediate);
        });
        return;
    }

    TENGINE_ASSERT(child->parent(), "child node {} doesn't have a parent", child->name());
    child->parent().reset();
    child->tree_ = nullptr;
    parent->children_.erase(child->name_);
}

void NodeTree::beginModificationQueue() {
    TENGINE_ASSERT(updateState_ == UpdateState::Idle, "Tree state == Updating");
    updateState_ = UpdateState::Updating;
}

void NodeTree::endModificationQueue() {
    TENGINE_ASSERT(updateState_ == UpdateState::Updating, "Tree state == Idle");
    updateState_ = UpdateState::Idle;
}

void NodeTree::applyModifications() {
    TENGINE_ASSERT(updateState_ == UpdateState::Idle, "Tree state == Updating");
    for(auto&& m : modifications_) {
        m();
    }
    modifications_.clear();
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
