#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aUV;
uniform mat4 u_MVP;
uniform mat4 u_Model;
out vec3 vWorld;
out vec2 vUV;
void main()
{
   vUV = aUV;
   vWorld = (u_Model * vec4(aPos, 1.0)).xyz;
   gl_Position = u_MVP * vec4(aPos, 1.0);
}
