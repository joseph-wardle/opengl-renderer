#version 460 core

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
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

uniform PointLight uLight;
uniform PhongMaterial uMaterial;
uniform vec3 uViewPos;
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;

void main()
{
    vec3 base_diffuse = texture(uDiffuseMap, vUV).rgb;
    vec3 base_spec = texture(uSpecularMap, vUV).rgb;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLight.position - vWorldPos);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);

    float distance = length(uLight.position - vWorldPos);
    float attenuation = 1.0 / (uLight.constant + uLight.linear * distance + uLight.quadratic * distance * distance);
    vec3 light_color = uLight.color * uLight.intensity * attenuation;

    vec3 ambient = uMaterial.ambient * base_diffuse;
    vec3 diffuse = uMaterial.diffuse * diff * base_diffuse;
    vec3 specular = uMaterial.specular * spec * base_spec;

    vec3 color = (ambient + diffuse + specular) * light_color;
    FragColor = vec4(color, 1.0);
}
