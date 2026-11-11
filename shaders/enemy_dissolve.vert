#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aUV;
uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform vec3 u_DissolveOrigin;
out vec3 vWorld;
out vec3 vDissolvePosition;
out vec2 vUV;

void main() {
    vWorld = (u_Model * vec4(aPos, 1.0)).xyz;
    // One coordinate system for the entire mob, including its child blocks.
    vDissolvePosition = vWorld - u_DissolveOrigin;
    vUV = aUV;
    gl_Position = u_MVP * vec4(aPos, 1.0);
}
