#include "tengine/ns/node.hpp"

#include "tengine/asset_manager.hpp"
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
}

void Node::removeChild(const NodePtr& child) {
}

auto Node::children() const -> const std::unordered_map<std::string, NodePtr>& {
    return children_;
}

void Node::setParent(const NodePtr& parent) {
}

auto Node::parent() const -> NodePtr {
    return parent_.lock();
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

void Node::enterTree() {
    enterTreeInternal();
}

void Node::exitTree() {
    exitTreeInternal();
}

void Node::ready() {
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
