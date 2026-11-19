#version 330 core
in vec2 vUV;
in vec3 vWorld;
out vec4 FragColor;
uniform vec3 u_Color;
uniform sampler2D u_Tex;
uniform bool u_UseTex;
uniform bool u_Lighting;
uniform float u_HitFlash;
uniform float u_Alpha;
uniform vec3 u_SunDir;
uniform mat4 u_LightVP;
uniform sampler2D u_ShadowMap;
uniform bool u_Shadows;
float shadowAt(vec3 world, vec3 n) {
   if (!u_Shadows) return 0.0;
   vec4 p = u_LightVP * vec4(world + n * 0.05, 1.0);
   vec3 q = p.xyz / p.w * 0.5 + 0.5;
   if (q.z > 1.0) return 0.0;
   vec2 texel = 1.0 / vec2(textureSize(u_ShadowMap, 0));
   float shadow = 0.0;
   for (int x = -1; x <= 1; ++x)
       for (int y = -1; y <= 1; ++y)
           shadow += (q.z - 0.001 > textureLod(u_ShadowMap, q.xy + vec2(x, y) * texel, 0.0).r) ? 1.0 : 0.0;
   return shadow / 9.0;
}
void main() {
   vec3 c = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;
   c = mix(c, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));
   if (u_Lighting) {
       vec3 n = normalize(cross(dFdx(vWorld), dFdy(vWorld)));
       float diffuse = max(dot(n, normalize(u_SunDir)), 0.0);
       c *= 0.45 + 0.55 * diffuse * (1.0 - 0.8 * shadowAt(vWorld, n));
   }
   FragColor = vec4(c, u_Alpha);
}
