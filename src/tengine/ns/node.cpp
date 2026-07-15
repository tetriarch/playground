#include "tengine/ns/node.hpp"

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

void Node::ready() {
    if(!scriptPath_.empty()) {
        // TODO: Do we want AssetManager to pre-load the file or do we wanna open the file via sol,
        // directly here..
        // scriptHandle_ = ScriptSystem->get()->doFile(scriptPath);
    }
}

void Node::enterTree() {
    onEnterTree();
}

void Node::exitTree() {
    onExitTree();
}

auto Node::children() const -> const std::unordered_map<std::string, NodePtr>& {
    return children_;
}

}  // namespace tengine
