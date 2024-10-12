#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec2 texCoords;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;
  vec3 color;
} push;

const float tiling = 1.0;

void main() {
  gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
  texCoords = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5) * tiling;
}
