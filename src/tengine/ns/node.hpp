#pragma once

#include "tengine/aliases.hpp"
#include "tengine/script_system.hpp"

namespace tengine {

class NodeTree;

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

    void setTree(NodeTree* tree);

public:
    // engine only
    // called when loading a scene
    void load();

    // scriptable
    virtual void ready();
    virtual void update(f32 dt);
    virtual void postUpdate(f32 dt);

    // engine only
    virtual void render() {};

protected:
    // these are implemented by derived classes
    virtual void loadInternal() {};
    virtual void readyInternal() {};

private:
    std::string name_;
    std::string scriptPath_;
    ScriptInstance script_;
    ScriptFunction readyFn_;
    ScriptFunction updateFn_;
    ScriptFunction postUpdateFn_;

private:
    friend class NodeTree;
    NodeHandle parent_;
    std::unordered_map<std::string, NodePtr> children_;
    NodeTree* tree_;
};

}  // namespace tengine
