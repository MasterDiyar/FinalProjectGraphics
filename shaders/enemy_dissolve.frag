#version 330 core
in vec3 vWorld;
in vec3 vDissolvePosition;
in vec2 vUV;
out vec4 FragColor;
uniform vec3 u_Color;
uniform sampler2D u_Tex;
uniform bool u_UseTex;
uniform float u_HitFlash;
uniform float u_DissolveProgress;
uniform bool u_DepthOnly;
uniform vec3 u_SunDir;
uniform mat4 u_LightVP;
uniform sampler2D u_ShadowMap;

float hash(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

float noise3D(vec3 p) {
    vec3 cell = floor(p);
    vec3 t = fract(p);
    t = t * t * (3.0 - 2.0 * t);
    return mix(
        mix(mix(hash(cell), hash(cell + vec3(1,0,0)), t.x),
            mix(hash(cell + vec3(0,1,0)), hash(cell + vec3(1,1,0)), t.x), t.y),
        mix(mix(hash(cell + vec3(0,0,1)), hash(cell + vec3(1,0,1)), t.x),
            mix(hash(cell + vec3(0,1,1)), hash(cell + vec3(1,1,1)), t.x), t.y), t.z);
}

float shadowAt(vec3 world, vec3 n) {
    vec4 p = u_LightVP * vec4(world + n * 0.05, 1.0);
    vec3 q = p.xyz / p.w * 0.5 + 0.5;
    if (q.z > 1.0) return 0.0;
    vec2 texel = 1.0 / vec2(textureSize(u_ShadowMap, 0));
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
            shadow += q.z - 0.001 > textureLod(u_ShadowMap, q.xy + vec2(x,y) * texel, 0.0).r ? 1.0 : 0.0;
    return shadow / 9.0;
}

void main() {
    // Derivatives precede discard so face normals remain valid at hole edges.
    vec3 n = normalize(cross(dFdx(vWorld), dFdy(vWorld)));
    float mask = noise3D(vDissolvePosition * 5.0 + vec3(17.3, 4.1, 9.7));
    float progress = clamp(u_DissolveProgress, 0.0, 1.0);
    if (progress >= 1.0 || (progress > 0.0 && mask < progress)) discard;
    // The shadow pass uses the very same mask, so holes also remove shadows.
    if (u_DepthOnly) { FragColor = vec4(0.0); return; }

    vec3 color = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;
    color = mix(color, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));
    float diffuse = max(dot(n, normalize(u_SunDir)), 0.0);
    color *= 0.45 + 0.55 * diffuse * (1.0 - 0.8 * shadowAt(vWorld, n));
    // Emissive orange/yellow band on surviving fragments beside the threshold.
    float edge = progress > 0.0 ? 1.0 - smoothstep(0.0, 0.09, mask - progress) : 0.0;
    color = mix(color, vec3(1.0, 0.48, 0.06), edge);
    FragColor = vec4(color, 1.0);
}
