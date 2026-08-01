#pragma once

#include "tengine/aliases.hpp"
#include "tengine/script_system.hpp"

namespace tengine {

class SceneTree;

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

    u32 depth() const;
    void setDepth(u32 depth);

public:
    void addChild(const NodePtr& child);
    void removeChild(const NodePtr& child);
    [[nodiscard]] auto children() const -> const std::unordered_map<std::string, NodePtr>&;

    void setParent(const NodePtr& parent);
    void resetParent();
    [[nodiscard]] Node* parent() const;

    void enterTree(SceneTree* tree);
    void exitTree();

public:
    // engine only
    // called when loading a scene
    void load();

    // scriptable
    void ready();
    void update(f32 dt);

    // engine only
    void render();

protected:
    // these are implemented by derived classes
    virtual void loadInternal() {};
    virtual void readyInternal() {};
    virtual void updateInternal(f32 dt) {};
    virtual void renderInternal() {};

private:
    std::string name_;
    std::string scriptPath_;
    u32 depth_;
    ScriptInstance script_;
    ScriptFunction readyFn_;
    ScriptFunction updateFn_;

private:
    Node* parent_;
    std::unordered_map<std::string, NodePtr> children_;
    SceneTree* tree_;

protected:
    bool isRenderable_;
};

}  // namespace tengine
