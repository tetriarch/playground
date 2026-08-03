#include "tengine/ns/node.hpp"

#include "tengine/asset_manager.hpp"
#include "tengine/ns/scene_tree.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

Node::Node() : name_("Node"), scriptPath_(""), depth_(0) {
}

Node::Node(const std::string& name) : name_(name), scriptPath_(""), depth_(0) {
}

auto Node::name() const -> const std::string& {
    return name_;
}

void Node::setName(const std::string& name) {
    name_ = name;
}

auto Node::scriptPath() const -> const std::string& {
    return scriptPath_;
}

void Node::setScriptPath(const std::string& scriptPath) {
    scriptPath_ = scriptPath;
}

u32 Node::depth() const {
    return depth_;
}

void Node::setDepth(u32 depth) {
    depth_ = depth;
}

void Node::addChild(const NodePtr& child) {
    TENGINE_ASSERT(child, "child is nullptr");
    TENGINE_ASSERT(child.get() != this, "node {} cannot be it's own child", name_);
    TENGINE_ASSERT(!child->parent(), "child node {} already has a parent", child->name());
    TENGINE_ASSERT(!child->tree_, "child {} already belongs to a tree", child->name());

    auto [_, inserted] = children_.emplace(child->name(), child);

    TENGINE_ASSERT(inserted, "node {} already has a child named {}", name_, child->name());
    // TODO: we should check inserted and if it is false
    // change the child's name with iterative number before trying to add it, instead of asserting
    // here (example: if parent has Child named Something it should become Something002, or similar,
    // it might be worth to enforce this with RegEx check

    child->setParent(shared_from_this());
    child->setDepth(depth_ + 1);

    if(tree_) {
        child->enterTree(tree_);
    }
}

void Node::removeChild(const NodePtr& child) {
    TENGINE_ASSERT(child, "child is nullptr");

    auto actualParent = child->parent();
    TENGINE_ASSERT(
        actualParent && actualParent == this, "node {} is not a child of {}", child->name(), name_
    );

    auto it = children_.find(child->name());
    TENGINE_ASSERT(
        it != children_.end() && it->second == child, "parent-child hierarchy is inconsistent"
    );

    child->resetParent();
    if(tree_) {
        child->enterTree(nullptr);
        return;
    }
    children_.erase(it);
}

auto Node::children() const -> const std::unordered_map<std::string, NodePtr>& {
    return children_;
}

void Node::setParent(const NodePtr& parent) {
    parent_ = parent.get();
}

void Node::resetParent() {
    parent_ = nullptr;
}

Node* Node::parent() const {
    return parent_;
}

void Node::enterTree(SceneTree* tree) {
    TENGINE_ASSERT(tree, "tree is nullptr");
    tree_ = tree;

    if(updateFn_) {
        tree_->registerForUpdate(shared_from_this());
    }

    if(isRenderable()) {
        tree_->registerForRender(shared_from_this());
    }

    for(auto&& [_, c] : children_) {
        c->enterTree(tree);
    }

    // since we call enter tree on children, the children are ready before parent
    ready();
}

void Node::exitTree() {
    TENGINE_ASSERT(tree_, "node {} is not part of a tree", name_);

    if(updateFn_) {
        tree_->unregisterForUpdate(shared_from_this());
    }

    for(auto&& [_, c] : children_) {
        c->exitTree();
    }

    tree_ = nullptr;
}

void Node::load() {
    if(!scriptPath_.empty()) {
        script_ = ScriptSystem::get()->runFile(scriptPath_);
        if(!script_) {
            auto scriptFullPath = AssetManager::get()->getAssetPath(scriptPath_).generic_string();
            // TODO: full Node path would be better here probably
            LOG_FATAL("Node", "failed to get script '{}' for '{}'", scriptFullPath, name_);
        }
    }

    auto ready = ScriptSystem::get()->function("ready", script_);
    if(ready) {
        readyFn_ = ready;
    }

    auto update = ScriptSystem::get()->function("update", script_);
    if(update) {
        updateFn_ = update;
    }

    // load derived of this class
    loadDerived();
}

void Node::ready() {
    TENGINE_ASSERT(tree_, "node {} is not part of a tree", name_);

    // ready derived of this class
    readyDerived();

    // finally call ready from script
    if(readyFn_) {
        auto result = readyFn_.value()(script_->table);
        auto validation = ScriptSystem::get()->validateExecution(result);
        if(!validation) {
            LOG_FATAL(
                "Node",
                "Lua runtime error\n"
                " Node: {}\n"
                " Callback: ready()\n"
                " Error: {}\n"
                " Source: {}\n",
                name_, validation.error,
                AssetManager::get()->getAssetPath(scriptPath_).generic_string()
            );
        }
    }
}

void Node::update(f32 dt) {
    TENGINE_ASSERT(tree_, "node {} is not part of a tree", name_);
    if(updateFn_) {
        // we call update with (self, dt) parameters
        auto result = updateFn_.value()(script_->table, dt);
        auto validation = ScriptSystem::get()->validateExecution(result);
        if(!validation) {
            LOG_FATAL(
                "Node",
                "Lua runtime error\n"
                " Node: '{}'\n"
                " Callback: update(dt)\n"
                " Error: {}\n"
                " Source: {}\n",
                name_, validation.error,
                AssetManager::get()->getAssetPath(scriptPath_).generic_string()
            );
        }
    }
}

void Node::render() {
    TENGINE_ASSERT(tree_, "node {} is not part of a tree", name_);

    renderDerived();
}

bool Node::isRenderable() const {
    return false;
}

}  // namespace tengine
