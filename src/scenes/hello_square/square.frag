#version 330 core
out vec4 FragColor;
uniform float uTime;

void main()
{
    float r = 0.5 + 0.5 * sin(uTime * 0.8);
    float g = 0.5 + 0.5 * sin(uTime * 1.3 + 2.0);
    float b = 0.5 + 0.5 * sin(uTime * 1.7 + 4.0);
    FragColor = vec4(r, g, b, 1.0);
}
