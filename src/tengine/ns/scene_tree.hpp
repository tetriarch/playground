/*
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 *
 */

#include "tengine/aliases.hpp"

#pragma once

namespace tengine {

class SceneTree {
public:
    SceneTree();

public:
    void setSceneRoot(const NodePtr& scene);
    void registerForUpdate(const NodePtr& node);
    void unregisterForUpdate(const NodePtr& node);
    void registerForRender(const NodePtr& node);
    void unregisterForRender(const NodePtr& node);
    void orderTreeUpdates();

public:
    void update(f32 dt);
    void render();

public:
    auto root() const -> const Node*;

private:
    NodePtr root_;
    std::vector<NodePtr> updateNodes_;
    std::vector<NodePtr> renderNodes_;
    bool unordered_;
};

}  // namespace tengine
