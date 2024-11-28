#version 450

layout (location = 0) in vec2 texCoords;
layout (location = 1) in vec4 particleColor;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 1) uniform sampler2D diffuseMap;

void main()
{
    vec4 diffuseColor = texture(diffuseMap, texCoords);
    outColor = diffuseColor * particleColor;
}
