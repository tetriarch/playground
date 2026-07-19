#include "tengine/ns/node.hpp"

#include "tengine/asset_manager.hpp"
#include "tengine/ns/node_tree.hpp"
#include "tengine/utils/logger.hpp"

namespace tengine {

Node::Node() : name_("Node"), scriptPath_("") {
}

Node::Node(const std::string& name) : name_(name), scriptPath_("") {
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

void Node::addChild(const NodePtr& child) {
    // TODO: children need to have unique names
    TENGINE_ASSERT(child, "child is nullptr");
    if(tree_) {
        tree_->addChild(shared_from_this(), child);
        return;
    }

    auto [it, _] = children_.emplace(child->name(), child);
    it->second->setParent(shared_from_this());
}

void Node::removeChild(const NodePtr& child) {
    TENGINE_ASSERT(child, "child is nullptr");
    if(tree_) {
        tree_->removeChild(shared_from_this(), child);
        return;
    }
    children_.erase(child->name());
}

auto Node::children() const -> const std::unordered_map<std::string, NodePtr>& {
    return children_;
}

void Node::setParent(const NodePtr& parent) {
    parent_ = parent;
}

auto Node::parent() const -> NodePtr {
    return parent_.lock();
}

void Node::setTree(NodeTree* tree) {
    // TODO: full Node path would be better here probably
    TENGINE_ASSERT(tree_ != tree, "node '{}' is already attached to a tree", name());
    tree_ = tree;
    for(auto&& c : children_) {
        c.second->setTree(tree);
    }
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

    loadInternal();
}

void Node::ready() {
    readyInternal();

    if(readyFn_) {
        readyFn_.value();
    }
}

void Node::update(f32 dt) {
    if(updateFn_) {
        // we call update with (self, dt) parameters
        updateFn_.value()(script_, dt);
    }
}

void Node::postUpdate(f32 dt) {
    if(postUpdateFn_) {
        // we call postUpdate with (self, dt) parameters
        postUpdateFn_.value()(script_, dt);
    }
}

}  // namespace tengine
