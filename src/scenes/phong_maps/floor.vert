#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV; // unused, but kept for shared layout

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec3 vWorldPos;

void main()
{
    vec4 world_pos = uModel * vec4(aPos, 1.0);
    vWorldPos = world_pos.xyz;
    vNormal = normalize(uNormalMatrix * aNormal);
    gl_Position = uProjection * uView * world_pos;
}
