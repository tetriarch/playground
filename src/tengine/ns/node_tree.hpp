/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/aliases.hpp"

#pragma once

// TODO:
// [x] root_ should hold only one scene as is child
// [ ] root_ cannot have parent
// [ ] root_ should probably be some sort of specialized RootNode final : public Node
// [x] deferred tree modifications

namespace tengine {

enum class UpdateState {
    Idle,
    Updating
};

enum class UpdateType {
    Deferred,
    Immediate
};

class NodeTree {
public:
    NodeTree();
    void setSceneRoot(const NodePtr& scene);

    // for deffered tree modifications
public:
    void addChild(
        const NodePtr& parent, const NodePtr& child, UpdateType updateType = UpdateType::Deferred
    );
    void removeChild(
        const NodePtr& parent, const NodePtr& child, UpdateType updateType = UpdateType::Deferred
    );
    void beginModificationQueue();
    void endModificationQueue();
    void applyModifications();

public:
    void ready();
    void update(f32 dt);
    void postUpdate(f32 dt);
    void render();

public:
    auto root() const -> const Node*;

private:
    NodePtr root_;
    UpdateState updateState_;
    std::vector<std::function<void()>> modifications_;
};

}  // namespace tengine
