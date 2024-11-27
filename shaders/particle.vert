#version 450

layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec4 particleColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
} ubo;

layout(push_constant) uniform Push {
  vec2 position;
  vec4 color;
} push;

const float tiling = 1.0;

void main()
{
    float scale = 10.0f;
    texCoords = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5) * tiling;
    particleColor = push.color;
    gl_Position = ubo.projection * vec4((position.xy * scale) + push.position, 0.0, 1.0);
}