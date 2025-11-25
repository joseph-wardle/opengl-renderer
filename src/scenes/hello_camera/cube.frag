#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform sampler2D uTex1;
uniform float uBlend;

void main()
{
    vec4 a = texture(uTex0, vUV);
    vec4 b = texture(uTex1, vUV);
    FragColor = mix(a, b, clamp(uBlend, 0.0, 1.0));
}
