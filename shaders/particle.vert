#version 450

layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec4 particleColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
} ubo;

layout(push_constant) uniform Push {
  vec2 transform; 
  float padding[2];
  vec4 color;
} push;

const float tiling = 1.0;

void main()
{
    float scale = 1.0f;
    texCoords = vec2(push.transform.x/2.0 + 0.5, push.transform.y/2.0 + 0.5) * tiling;
    particleColor = push.color;
    gl_Position = ubo.projection * vec4((push.transform.xy * scale) + push.transform, 0.0, 1.0);
}