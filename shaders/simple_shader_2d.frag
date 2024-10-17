#version 450

layout (location = 0) in vec2 texCoords;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 1) uniform sampler2D diffuseMap;

layout(push_constant) uniform Push {
  mat2 transform;
  vec3 offset;
  vec3 color;
} push;

void main() {
  // Sample the color from the diffuse map
  vec4 diffuseColor = texture(diffuseMap, texCoords);
  vec4 finalColor = diffuseColor * vec4(push.color, 1.0);
  // outColor = vec4(push.color, 1.0);
  outColor = finalColor;
}
