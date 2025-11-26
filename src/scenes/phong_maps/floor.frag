#version 460 core

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float inner_cos;
    float outer_cos;
    float constant;
    float linear;
    float quadratic;
};

struct PhongMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 vNormal;
in vec3 vWorldPos;

layout(location = 0) out vec4 FragColor;

const int MAX_DIR = 4;
const int MAX_POINT = 4;
const int MAX_SPOT = 2;

uniform int uDirCount;
uniform int uPointCount;
uniform int uSpotCount;
uniform DirectionalLight uDirLights[MAX_DIR];
uniform PointLight      uPointLights[MAX_POINT];
uniform SpotLight       uSpotLights[MAX_SPOT];
uniform PhongMaterial   uMaterial;
uniform vec3            uViewPos;
uniform vec3            uColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 color = vec3(0.0);

    for (int i = 0; i < uDirCount && i < MAX_DIR; ++i) {
        DirectionalLight light = uDirLights[i];
        if (light.intensity <= 0.0) continue;
        vec3 L = normalize(-light.direction);
        vec3 H = normalize(L + V);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        vec3 light_color = light.color * light.intensity;
        color += (uMaterial.ambient * uColor + uMaterial.diffuse * diff * uColor + uMaterial.specular * spec) * light_color;
    }

    for (int i = 0; i < uPointCount && i < MAX_POINT; ++i) {
        PointLight light = uPointLights[i];
        if (light.intensity <= 0.0) continue;
        vec3 L = normalize(light.position - vWorldPos);
        vec3 H = normalize(L + V);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        float distance = length(light.position - vWorldPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
        vec3 light_color = light.color * light.intensity * attenuation;
        color += (uMaterial.ambient * uColor + uMaterial.diffuse * diff * uColor + uMaterial.specular * spec) * light_color;
    }

    for (int i = 0; i < uSpotCount && i < MAX_SPOT; ++i) {
        SpotLight light = uSpotLights[i];
        if (light.intensity <= 0.0) continue;
        vec3 L = normalize(light.position - vWorldPos);
        vec3 H = normalize(L + V);
        float theta = dot(L, normalize(-light.direction));
        float epsilon = light.inner_cos - light.outer_cos;
        float cutoff = clamp((theta - light.outer_cos) / epsilon, 0.0, 1.0);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        float distance = length(light.position - vWorldPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
        vec3 light_color = light.color * light.intensity * attenuation * cutoff;
        color += (uMaterial.ambient * uColor + uMaterial.diffuse * diff * uColor + uMaterial.specular * spec) * light_color;
    }

    FragColor = vec4(color, 1.0);
}
