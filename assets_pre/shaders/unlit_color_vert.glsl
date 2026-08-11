#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aUV;

layout (location = 0) out vec4 vColor;
layout (location = 1) out vec2 vUV;

layout (set = 1, binding = 0) uniform TransformsBlock {
    mat4 model;
    mat4 view;
    mat4 proj;
} aTransforms;

void main() {
    gl_Position = aTransforms.proj * aTransforms.view * aTransforms.model * vec4(aPosition, 1.0);
    vColor = aColor;
    vUV = aUV;
}
