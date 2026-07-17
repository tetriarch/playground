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
    void addChild(const NodePtr& child);
    void removeChild(const NodePtr& child);
    [[nodiscard]] auto children() const -> const std::unordered_map<std::string, NodePtr>&;

    void setParent(const NodePtr& parent);
    [[nodiscard]] auto parent() const -> NodePtr;

public:
    // engine only
    // called when loading a scene
    void load();

    void enterTree();
    void exitTree();

    // scriptable
    virtual void ready();
    virtual void update(f32 dt);
    virtual void postUpdate(f32 dt);

    // engine only
    virtual void render() {};

protected:
    // these are implemented by derived classes
    virtual void loadInternal() {};
    virtual void enterTreeInternal() {};
    virtual void exitTreeInternal() {};

private:
    std::string name_;
    std::string scriptPath_;
    ScriptInstance script_;
    ScriptFunction readyFn_;
    ScriptFunction updateFn_;
    ScriptFunction postUpdateFn_;

private:
    NodeHandle parent_;
    std::unordered_map<std::string, NodePtr> children_;
};

}  // namespace tengine
