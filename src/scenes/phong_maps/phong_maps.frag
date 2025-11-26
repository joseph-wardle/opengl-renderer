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
in vec2 vUV;

layout(location = 0) out vec4 FragColor;

uniform DirectionalLight uDirLight;
uniform PointLight      uPointLight;
uniform SpotLight       uSpotLight;
uniform PhongMaterial uMaterial;
uniform vec3 uViewPos;
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;

void main()
{
    vec3 base_diffuse = texture(uDiffuseMap, vUV).rgb;
    vec3 base_spec = texture(uSpecularMap, vUV).rgb;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 color = vec3(0.0);

    // Directional
    if (uDirLight.intensity > 0.0) {
        vec3 L = normalize(-uDirLight.direction);
        vec3 H = normalize(L + V);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        vec3 light_color = uDirLight.color * uDirLight.intensity;
        color += (uMaterial.ambient * base_diffuse + uMaterial.diffuse * diff * base_diffuse + uMaterial.specular * spec * base_spec) * light_color;
    }

    // Point
    if (uPointLight.intensity > 0.0) {
        vec3 L = normalize(uPointLight.position - vWorldPos);
        vec3 H = normalize(L + V);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        float distance = length(uPointLight.position - vWorldPos);
        float attenuation = 1.0 / (uPointLight.constant + uPointLight.linear * distance + uPointLight.quadratic * distance * distance);
        vec3 light_color = uPointLight.color * uPointLight.intensity * attenuation;
        color += (uMaterial.ambient * base_diffuse + uMaterial.diffuse * diff * base_diffuse + uMaterial.specular * spec * base_spec) * light_color;
    }

    // Spot
    if (uSpotLight.intensity > 0.0) {
        vec3 L = normalize(uSpotLight.position - vWorldPos);
        vec3 H = normalize(L + V);
        float theta = dot(L, normalize(-uSpotLight.direction));
        float epsilon = uSpotLight.inner_cos - uSpotLight.outer_cos;
        float intensity = clamp((theta - uSpotLight.outer_cos) / epsilon, 0.0, 1.0);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        float distance = length(uSpotLight.position - vWorldPos);
        float attenuation = 1.0 / (uSpotLight.constant + uSpotLight.linear * distance + uSpotLight.quadratic * distance * distance);
        vec3 light_color = uSpotLight.color * uSpotLight.intensity * attenuation * intensity;
        color += (uMaterial.ambient * base_diffuse + uMaterial.diffuse * diff * base_diffuse + uMaterial.specular * spec * base_spec) * light_color;
    }

    FragColor = vec4(color, 1.0);
}
