/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/aliases.hpp"

#pragma once

namespace tengine {

enum class TreeState {
    Idle,
    GettingReady,
    Updating
};

class SceneTree {
public:
    SceneTree();

public:
    void setSceneRoot(const NodePtr& scene);
    void addChild(const NodePtr& parent, const NodePtr& child);
    void removeChild(const NodePtr& parent, const NodePtr& child);

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
    void readyImmediate();
    void setSceneRootImmediate(const NodePtr& scene);
    void addChildImmediate(const NodePtr& parent, const NodePtr& child);
    void removeChildImmediate(const NodePtr& parent, const NodePtr& child);
    void readyNodeImmediate(const NodePtr& node);

private:
    bool active_;
    NodePtr root_;
    TreeState state_;
    std::vector<std::function<void()>> modifications_;
};

}  // namespace tengine
