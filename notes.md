# Node creation/addition cases

- loading a scene/subscene
- creating a node on the fly - though creating node on the fly usually means loading a scene/subscene
- swapping scenes/subscenes - remove old one, load new one

# Loading a scene
- SceneLoader loads a scene -> Scene
- Scene added to SceneTree
- When Scene added to the SceneTree -> Node::enterTree()
- Whene Scene/Node leaves the SceneTree -> Node::leaveTree()
- ? How to cascade additon of nodes to the tree?
- ? How to tell whether or not a node needs to be rendered?

# tree modification during game loop
- addition/removal of nodes mustn't break container and/or iterator

# TODO
 - Nodes call ready twice, fix it
