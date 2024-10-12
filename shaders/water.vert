#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 clipSpace;
layout(location = 1) out vec2 texCoords;
layout(location = 2) out vec3 toCameraVector;
layout(location = 3) out vec3 fromLightVector;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec3 cameraPos;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(set = 1, binding = 0) uniform GameObjectBufferData {
    mat4 modelMatrix;
    mat4 normalMatrix;
} gameObject;

layout(push_constant) uniform Push {
  vec4 position;
  vec4 rotation;
  vec4 scale;
} push;

const float tiling = 4.0;

void main() {
    vec4 positionWorld = gameObject.modelMatrix * vec4(position, 1.0);
    clipSpace = ubo.projection * ubo.view * positionWorld;
    gl_Position = clipSpace;
    texCoords = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5) * tiling;
    toCameraVector = ubo.cameraPos - positionWorld.xyz;
    fromLightVector = positionWorld.xyz - ubo.pointLights[0].position.xyz;
}