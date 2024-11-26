#pragma once

#include "nile_model.hpp"
#include "nile_swap_chain.hpp"
#include "nile_texture.hpp"
#include "nile_off_screen.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace nile{

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();

  glm::mat3 normalMatrix();
};

struct TransformComponent2d {
  glm::vec3 translation{};
  glm::vec2 scale{1.f, 1.f};
  float rotation{};

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMatrix{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMatrix * scaleMat;
  }
};

struct ParticleComponent 
{
  glm::vec2 Position{0.0f};
  glm::vec2 Velocity;
  glm::vec4 Color{1.0f};
  float     Life = 0.0f;
};

struct BallComponent
{
    bool      stuck = true;
    float     radius;
    glm::vec2 Move(float dt, unsigned int window_width);
    void      reset(glm::vec2 position, glm::vec2 velocity);
};

struct WaterComponent
{
  float rippleIntensity = 1.0f;
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
};

struct RigidBodyComponent2d {
  glm::vec2 velocity;
  float mass{1.0f};
};

struct GameObjectBufferData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

class NileGameObjectManager;    // forward declare game object manager class

class NileGameObject {
 public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, NileGameObject>;

  NileGameObject(NileGameObject &&) = default;
  NileGameObject(const NileGameObject &) = delete;
  NileGameObject &operator=(const NileGameObject &) = delete;
  NileGameObject &operator=(NileGameObject &&) = delete;

  id_t getId() { return id; }
  
  void setIsHidden(bool set_show_model) { isHidden = set_show_model; }
  bool getIsHidden() { return isHidden; }

  VkDescriptorBufferInfo getBufferInfo(int frameIndex);

  glm::vec3 color{};
  bool isSolid = false;
  bool isMirror = false;
  bool isParticle = false;
  bool Destroyed = false;
  bool isVectorField = false;
  
  TransformComponent transform{};
  TransformComponent2d transform2d{};
  RigidBodyComponent2d rigidBody2d{};

  // Optional pointer components
  std::shared_ptr<NileModel> model{};
  std::shared_ptr<NileTexture> diffuseMap = nullptr;
  std::shared_ptr<NileTexture> normalMap = nullptr;
  std::shared_ptr<NileTexture> depthMap = nullptr;
  std::shared_ptr<NileTexture> dudvMap = nullptr;
  std::shared_ptr<NileOffScreen> reflectionTexture = nullptr;
  std::shared_ptr<NileOffScreen> refractionTexture = nullptr;
  std::unique_ptr<PointLightComponent> pointLight = nullptr;
  std::unique_ptr<WaterComponent> water = nullptr;
  std::unique_ptr<BallComponent> ball = nullptr;
  std::unique_ptr<ParticleComponent> particle = nullptr;
  
 private:
  NileGameObject(id_t objId, const NileGameObjectManager &manager);

  id_t id;
  bool isHidden = false;

  const NileGameObjectManager &gameObjectManager;

  friend class NileGameObjectManager;
};

struct GameLevel
{
void load(
    NileDevice& device, 
    NileGameObjectManager& gom, 
    const std::string& file, 
    unsigned int levelWidth, 
    unsigned int levelHeight,
    bool isCurrentLevel = false);
bool isCompleted();

void init(
    NileDevice& device, 
    NileGameObjectManager& gom, 
    const std::vector<std::vector<unsigned int>>& tileData,
    unsigned int levelWidth, unsigned int levelHeight, bool isCurrentLevel);
    std::vector<NileGameObject::id_t> bricks;
};

class NileGameObjectManager {
  public:
   static constexpr int MAX_GAME_OBJECTS = 1000;

   NileGameObjectManager(NileDevice &device);
   NileGameObjectManager(const NileGameObjectManager &) = delete;
   NileGameObjectManager &operator=(const NileGameObjectManager &) = delete;
   NileGameObjectManager(NileGameObjectManager &&) = delete;
   NileGameObjectManager &operator=(NileGameObjectManager &&) = delete;

   NileGameObject &createGameObject() {
    assert(currentId < MAX_GAME_OBJECTS && "Max game object count exceeded!");
    auto gameObject = NileGameObject{currentId++, *this};
    auto gameObjectId = gameObject.getId();
    gameObject.diffuseMap = textureDefault;
    gameObject.normalMap = textureDefault;
    gameObject.depthMap = textureDefault;
    gameObject.dudvMap = dudvDefault;
    gameObjects.emplace(gameObjectId, std::move(gameObject));
    return gameObjects.at(gameObjectId);
   }

   NileGameObject &makePointLight(
      float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

   NileGameObject &makeBall(float radius = .025f, glm::vec2 velocity = {.05f, -.05f});

   NileGameObject &makeWater(float rippleIntensity = 1.f);

   NileGameObject &makeParticle(
    float Life = 0.0f, glm::vec2 Position = glm::vec2(0.0f), glm::vec2 Velocity = glm::vec2(0.0f), glm::vec4 Color = glm::vec4(1.0f));

   VkDescriptorBufferInfo getBufferInfoForGameObject(
        int frameIndex, NileGameObject::id_t gameObjectId) const {
      return uboBuffers[frameIndex]->descriptorInfoForIndex(gameObjectId);
    }

    void updateBuffer(int frameIndex);

    NileGameObject::Map gameObjects{};
    std::vector<std::unique_ptr<NileBuffer>> uboBuffers{NileSwapChain::MAX_FRAMES_IN_FLIGHT};

    private:
    NileGameObject::id_t currentId = 0;
    std::shared_ptr<NileTexture> textureDefault;
    std::shared_ptr<NileTexture> dudvDefault;
};

}  // namespace nile
