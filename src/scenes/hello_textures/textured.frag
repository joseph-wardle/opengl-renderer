#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform sampler2D uTex1;
uniform float uBlend;

void main()
{
    vec4 c0 = texture(uTex0, vUV);
    vec4 c1 = texture(uTex1, vUV);
    FragColor = mix(c0, c1, clamp(uBlend, 0.0, 1.0));
}
