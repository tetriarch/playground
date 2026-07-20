#include "tengine/ns/node.hpp"

#include "tengine/asset_manager.hpp"
#include "tengine/ns/scene_tree.hpp"
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
    TENGINE_ASSERT(child, "child is nullptr");
    // TODO: children need to have unique names
    if(tree_) {
        tree_->addChild(shared_from_this(), child);
        return;
    }

    children_.emplace(child->name(), child);
    child->setParent(shared_from_this());
}

void Node::removeChild(const NodePtr& child) {
    TENGINE_ASSERT(child, "child is nullptr");
    if(tree_) {
        tree_->removeChild(shared_from_this(), child);
        return;
    }
    child->setParent(nullptr);
    children_.erase(child->name());
}

auto Node::children() const -> const std::unordered_map<std::string, NodePtr>& {
    return children_;
}

void Node::setParent(const NodePtr& parent) {
    parent_ = parent;
}

void Node::resetParent() {
    parent_.reset();
}

auto Node::parent() const -> NodePtr {
    return parent_.lock();
}

void Node::setTree(SceneTree* tree) {
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
    // ready children first
    for(auto&& c : children_) {
        c.second->ready();
    }

    // ready the descendant
    readyInternal();

    // finaly call ready from script
    if(readyFn_) {
        readyFn_.value();
    }
    std::println("{}, ready", name_);
}

void Node::update(f32 dt) {
    if(updateFn_) {
        // we call update with (self, dt) parameters
        updateFn_.value()(script_, dt);
    }

    updateInternal(dt);

    for(auto&& c : children_) {
        c.second->update(dt);
    }
    std::println("{}, update", name_);
}

void Node::postUpdate(f32 dt) {
    if(postUpdateFn_) {
        // we call postUpdate with (self, dt) parameters
        postUpdateFn_.value()(script_, dt);
    }

    postUpdateInternal(dt);

    for(auto&& c : children_) {
        c.second->postUpdate(dt);
    }

    std::println("{}, postUpdate", name_);
}

void Node::render() {
    renderInternal();

    for(auto&& c : children_) {
        c.second->render();
    }
    std::println("{}, render", name_);
}

}  // namespace tengine
