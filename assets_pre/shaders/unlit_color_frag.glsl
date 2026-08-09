#version 460 core

layout (location = 0) in vec4 vColor;
layout (location = 1) in vec2 vUV;
layout (location = 0) out vec4 fColor;

layout(set = 2, binding = 0) uniform sampler2D hello_world;

void main() {
    fColor = texture(hello_world, vUV); 
}
