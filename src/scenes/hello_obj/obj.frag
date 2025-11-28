#version 330 core
in vec3 vNormal;
in vec3 vFragPos;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uViewPos;
uniform vec3 uBaseColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    float diff = max(dot(N, L), 0.0);

    vec3 ambient = 0.15 * uLightColor * uBaseColor;
    vec3 diffuse = diff * uLightColor * uBaseColor;

    FragColor = vec4(ambient + diffuse, 1.0);
}
