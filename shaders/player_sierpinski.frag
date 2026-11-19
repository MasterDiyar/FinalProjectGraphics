#version 330 core
in vec3 vWorld;
in vec3 LocalPos;
in vec2 vUV;
in vec3 vNormal;

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
uniform bool u_UseNoise;

float hash31(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float valueNoise3D(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    float n000 = hash31(i + vec3(0.0, 0.0, 0.0));
    float n100 = hash31(i + vec3(1.0, 0.0, 0.0));
    float n010 = hash31(i + vec3(0.0, 1.0, 0.0));
    float n110 = hash31(i + vec3(1.0, 1.0, 0.0));
    float n001 = hash31(i + vec3(0.0, 0.0, 1.0));
    float n101 = hash31(i + vec3(1.0, 0.0, 1.0));
    float n011 = hash31(i + vec3(0.0, 1.0, 1.0));
    float n111 = hash31(i + vec3(1.0, 1.0, 1.0));

    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);

    float nxy0 = mix(nx00, nx10, f.y);
    float nxy1 = mix(nx01, nx11, f.y);

    return mix(nxy0, nxy1, f.z);
}

float fbmNoise(vec3 p) {
    float sum = 0.0;
    float amp = 0.5;
    float freq = 1.0;
    for (int i = 0; i < 4; ++i) {
        sum += amp * valueNoise3D(p * freq);
        freq *= 2.03;
        amp *= 0.5;
    }
    return sum;
}

float shadowAt(vec3 world, vec3 n) {
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
    vec3 baseColor = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;

    if (u_UseNoise) {
        float n = fbmNoise(vec3(vWorld.x * 4.5, 0.0, vWorld.z * 4.5));
        n = smoothstep(0.25, 0.75, n);

        float grain = mix(0.75, 1.12, n);
        baseColor *= grain;

        float edge = clamp(fwidth(n) * 4.0, 0.0, 1.0);
        baseColor *= mix(1.0, 0.7, edge);
    }

    vec3 c = mix(baseColor, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));

    if (u_Lighting) {
        vec3 nrm = normalize(vNormal);
        float diffuse = max(dot(nrm, normalize(u_SunDir)), 0.0);
        c *= 0.45 + 0.55 * diffuse * (1.0 - 0.8 * shadowAt(vWorld, nrm));
    }

    FragColor = vec4(c, u_Alpha);
}