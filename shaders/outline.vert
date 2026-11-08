#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_MVP;
uniform float u_OutlineWidth; 

void main() {
    vec3 scaledPos = aPos + aNormal * u_OutlineWidth;
    gl_Position = u_MVP * vec4(scaledPos, 1.0);
}