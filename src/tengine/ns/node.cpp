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

}  // namespace tengine
