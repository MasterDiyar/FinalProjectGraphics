#version 330 core
in vec3 vNormal;
in vec2 vUV;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 u_Color;
uniform sampler2D u_Tex;
uniform bool u_UseTex;

void main() {
    // Безопасно выбираем базовый цвет: если текстура есть — семплируем, если нет — используем u_Color
    vec3 baseColor = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.7));
    float NdotL = dot(normalize(vNormal), lightDir);
    
    // Cel-Shading квантование
    float intensity = smoothstep(0.0, 0.05, NdotL);
    if (intensity > 0.6) {
        intensity = 1.0;
    } else if (intensity > 0.1) {
        intensity = 0.7;
    } else {
        intensity = 0.4;
    }

    // Краевое свечение (Rim light)
    vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0)); 
    float rim = 1.0 - max(dot(viewDir, normalize(vNormal)), 0.0);
    rim = smoothstep(0.6, 0.9, rim) * 0.3; 

    vec3 finalColor = baseColor * intensity + vec3(rim);
    FragColor = vec4(finalColor, 1.0);
}