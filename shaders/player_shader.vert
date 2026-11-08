#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

out vec3 vNormal;
out vec2 vUV;
out vec3 vWorldPos;

void main() {
    vUV = aUV;
    vNormal = normalize(u_NormalMatrix * aNormal);
    vWorldPos = vec3(u_Model * vec4(aPos, 1.0));
    gl_Position = u_MVP * vec4(aPos, 1.0);
}