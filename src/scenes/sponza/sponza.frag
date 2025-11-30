#version 330 core
in vec3 vNormal;
in vec3 vFragPos;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uViewPos;
uniform vec3 uBaseColor;
uniform bool uHasDiffuseMap;
uniform bool uHasSpecularMap;
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uViewPos - vFragPos);
    vec3 H = normalize(L + V);

    vec3 albedo = uBaseColor;
    float alpha = 1.0;
    if (uHasDiffuseMap) {
        vec4 sampled = texture(uDiffuseMap, vUV);
        albedo = sampled.rgb;
        alpha = sampled.a;
    }
    if (alpha <= 0.001) {
        discard;
    }

    float diff = max(dot(N, L), 0.0);

    float spec = 0.0;
    if (diff > 0.0) {
        float spec_strength = 0.5;
        float shininess = 32.0;
        if (uHasSpecularMap) {
            spec_strength = texture(uSpecularMap, vUV).r;
        }
        spec = spec_strength * pow(max(dot(N, H), 0.0), shininess);
    }

    vec3 ambient = 0.15 * albedo;
    vec3 diffuse = diff * albedo * uLightColor;
    vec3 specular = spec * uLightColor;

    FragColor = vec4(ambient + diffuse + specular, alpha);
}
