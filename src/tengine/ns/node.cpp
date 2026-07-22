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
    TENGINE_ASSERT(child.get() != this, "node {} cannot be it's own child", name_);
    TENGINE_ASSERT(
        !child->isAncestorOf(shared_from_this()), "adding {} under {} would create hierarchy cycle",
        child->name(), name_
    );
    TENGINE_ASSERT(!child->parent(), "child node {} already has a parent", child->name());
    TENGINE_ASSERT(!child->tree_, "child {} already belongs to a tree", child->name());

    if(tree_) {
        tree_->addChild(shared_from_this(), child);
        return;
    }

    auto [_, inserted] = children_.emplace(child->name(), child);

    TENGINE_ASSERT(inserted, "node {} already has a child named {}", name_, child->name());
    // TODO: we should check inserted and if it is false
    // change the child's name with iterative number before trying to add it, instead of asserting
    // here (example: if parent has Child named Something it should become Something002, or similar,
    // it might be worth to enforce this with RegEx check

    child->setParent(shared_from_this());
}

void Node::removeChild(const NodePtr& child) {
    TENGINE_ASSERT(child, "child is nullptr");

    if(tree_) {
        tree_->removeChild(shared_from_this(), child);
        return;
    }

    auto actualParent = child->parent();

    TENGINE_ASSERT(
        actualParent && actualParent.get() == this, "node {} is not a child of {}", child->name(),
        name_
    );

    auto it = children_.find(child->name());

    TENGINE_ASSERT(
        it != children_.end() && it->second == child, "parent-child hierarchy is inconsistent"
    );

    child->resetParent();
    children_.erase(it);
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

bool Node::isAncestorOf(const NodePtr& node) const {
    auto current = node->parent();
    while(current) {
        if(current.get() == this) {
            return true;
        }
        current = current->parent();
    }
    return false;
}

void Node::setTree(SceneTree* tree) {
    tree_ = tree;

    for(auto&& [_, c] : children_) {
        c->setTree(tree);
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
    TENGINE_ASSERT(tree_, "node is not part of a tree");

    // ready children first
    for(auto&& [_, c] : children_) {
        c->ready();
    }
    std::println("{}, ready", name_);

    // ready derived of this class
    readyInternal();

    // finaly call ready from script
    if(readyFn_) {
        readyFn_.value()(script_);
    }
}

void Node::update(f32 dt) {
    TENGINE_ASSERT(tree_, "node is not part of a tree");
    std::println("{}, update", name_);
    if(updateFn_) {
        // we call update with (self, dt) parameters
        updateFn_.value()(script_, dt);
    }

    updateInternal(dt);

    for(auto&& [_, c] : children_) {
        c->update(dt);
    }
}

void Node::postUpdate(f32 dt) {
    TENGINE_ASSERT(tree_, "node is not part of a tree");
    std::println("{}, postUpdate", name_);
    if(postUpdateFn_) {
        // we call postUpdate with (self, dt) parameters
        postUpdateFn_.value()(script_, dt);
    }

    postUpdateInternal(dt);

    for(auto&& [_, c] : children_) {
        c->postUpdate(dt);
    }
}

void Node::render() {
    TENGINE_ASSERT(tree_, "node is not part of a tree");
    std::println("{}, render", name_);
    renderInternal();

    for(auto&& [_, c] : children_) {
        c->render();
    }
}

}  // namespace tengine
