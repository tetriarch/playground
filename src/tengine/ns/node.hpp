#pragma once

#include "tengine/aliases.hpp"
#include "tengine/script_system.hpp"

namespace tengine {

class Node : public std::enable_shared_from_this<Node> {
public:
    Node();
    Node(const std::string& name);
    virtual ~Node() = default;

public:
    [[nodiscard]] auto name() const -> const std::string&;
    void setName(const std::string& name);
    [[nodiscard]] auto scriptPath() const -> const std::string&;
    void setScriptPath(const std::string& scriptPath);

public:
    void addChild(const NodePtr& child) {};
    void removeChild(const NodePtr& child) {};

public:
    // engine only
    void enterTree();
    void exitTree();
    virtual void onEnterTree() {};
    virtual void onExitTree() {};

    // scriptable
    virtual void ready();
    virtual void update(const f32 dt) {};
    virtual void postUpdate(const f32 dt) {};

    // engine only
    virtual void render() {};

public:
    auto children() const -> const std::unordered_map<std::string, NodePtr>&;

private:
    std::string name_;
    std::string scriptPath_;
    ScriptInstance scriptInstance_;
    NodeHandle parent_;
    std::unordered_map<std::string, NodePtr> children_;
};

}  // namespace tengine
