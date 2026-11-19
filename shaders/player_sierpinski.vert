#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_MVP;
uniform mat4 u_Model;

out vec3 vWorld;
out vec3 LocalPos;
out vec2 vUV;
out vec3 vNormal;

void main() {
    LocalPos = aPos; 
    vUV = aUV;
    vWorld = vec3(u_Model * vec4(aPos, 1.0));
    vNormal = mat3(transpose(inverse(u_Model))) * aNormal;
    gl_Position = u_MVP * vec4(aPos, 1.0);
}