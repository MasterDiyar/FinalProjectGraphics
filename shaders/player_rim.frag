#version 330 core
in vec2 vUV;
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 u_Color;
uniform sampler2D u_Tex;
uniform bool u_UseTex;
uniform float u_HitFlash;
uniform float u_Alpha;
uniform vec3 u_ViewPos;        // Позиция камеры в мировом пространстве

// Настройки Рим-света
uniform vec3 u_RimColor = vec3(0.2, 0.6, 1.0); // Цвет свечения (например, неоново-голубой)
uniform float u_RimPower = 3.0;                // Концентрация/резкость свечения (больше = тоньше контур)
uniform float u_RimIntensity = 1.5;            // Яркость свечения

void main() {
    // 1. Базовый цвет текстуры или цвета меша
    vec3 baseColor = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;
    
    // Эффект получения урона (если активирован хитфлэш)
    baseColor = mix(baseColor, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));

    // 2. Расчет векторов для Rim Lighting
    vec3 n = normalize(vNormal);
    vec3 viewDir = normalize(u_ViewPos - vWorldPos);

    // Интенсивность края: чем ближе угол к 90 градусам (касательная к взгляду), тем выше значение
    float rimFactor = 1.0 - max(dot(n, viewDir), 0.0);
    
    // Возводим в степень u_RimPower, чтобы сузить полоску свечения к самому краю
    float rim = pow(rimFactor, u_RimPower) * u_RimIntensity;

    // 3. Итоговое смешивание базового цвета и свечения по контуру
    vec3 finalColor = baseColor + (u_RimColor * rim);

    FragColor = vec4(finalColor, u_Alpha);
}