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
uniform vec3 u_ViewPos;

uniform vec3 u_RimColor;
uniform float u_RimPower;
uniform float u_RimIntensity;

void main() {
    vec3 baseColor = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;
    baseColor = mix(baseColor, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));

    vec3 n = normalize(vNormal);
    vec3 viewDir = normalize(u_ViewPos - vWorldPos);

    // Угол между нормалью и направлением на камеру
    float dotNV = dot(n, viewDir);
    
    // Отсекаем всё, что смотрит на камеру (оставляем только самый край, где dotNV близок к 0)
    float rimFactor = 1.0 - max(dotNV, 0.0);
    
    // Возводим в большую степень (например, 4.0 или 5.0), чтобы полоска была узкой
    float rim = pow(rimFactor, 4.0) * u_RimIntensity;

    // КРИТИЧЕСКИ ВАЖНО: 
    // Вместо сложения (+) мы используем интерполяцию или контролируемое добавление,
    // либо умножаем цвет текстуры на коэффициент свечения по краям, 
    // чтобы черные глаза оставались черными, а текстура не заливалась целиком.
    vec3 finalColor = baseColor + (u_RimColor * rim);

    // Жестко ограничиваем значения, чтобы пиксели не уходили в «белую кашу» (HDR клиппинг)
    finalColor = clamp(finalColor, 0.0, 1.0);

    FragColor = vec4(finalColor, u_Alpha);
}