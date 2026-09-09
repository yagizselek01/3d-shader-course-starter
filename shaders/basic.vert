#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 worldPosition;
out vec3 worldNormal;

void main()
{
    vec4 world = model * vec4(aPosition, 1.0);

    worldPosition = world.xyz;
    worldNormal = normalMatrix * aNormal;

    gl_Position = projection * view * world;
}