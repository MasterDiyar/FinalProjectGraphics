#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Если в модели/меше есть нормали
layout (location = 2) in vec2 aUV;

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    vUV = aUV;
    vWorldPos = vec3(u_Model * vec4(aPos, 1.0));
    vNormal = normalize(u_NormalMatrix * aNormal);
    gl_Position = u_MVP * vec4(aPos, 1.0);
}