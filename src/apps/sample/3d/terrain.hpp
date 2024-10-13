#include "apps/app.hpp"

#include "framework/systems/lights/point_light_system.hpp"
#include "framework/systems/terrain/terrain_system.hpp"

namespace nile{
class Terrain : public App3D {

public:
  Terrain();
  ~Terrain() override;

  void loop() override;
  void start() override;

private:
  void loadGameObjects() override;
  NileGameObject::id_t obj_id = 0;
};

Terrain::Terrain() { 
}

void Terrain::start() {
  loadGameObjects();
  loop();
}

Terrain::~Terrain() {}

void Terrain::loop() {
  RenderSystem3D renderSystem3D{
      nileDevice,
      nileRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightSystem{
      nileDevice,
      nileRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  auto& viewerObject = gameObjectManager.createGameObject();
  viewerObject.transform.translation.z = -2.5f;

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!nileWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.moveInPlaneXYZ(nileWindow.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = nileRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    // Start the DearImgui frame
    ui.startUI();

    if (auto commandBuffer = nileRenderer.beginFrame()) 
    {
      int frameIndex = nileRenderer.getFrameIndex();
      framePools[frameIndex]->resetPool();
      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          *framePools[frameIndex],
          gameObjectManager.gameObjects};

      // update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.inverseView = camera.getInverseView();
      if (ui.enable_point_lights) {
        pointLightSystem.update(frameInfo, ubo);
      }
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // final step of upate is updating the game objects buffer data
      // The render functions MUST not change a game objects transform data
      gameObjectManager.updateBuffer(frameIndex);

      // gameObjectManager.updateFromScene(obj_id, ui.terrain_pos, ui.terrain_rot, ui.set_show_model);

      // render
      nileRenderer.beginSwapChainRenderPass(commandBuffer);

      // order here matters
      renderSystem3D.renderGameObjects(frameInfo);

      if (ui.enable_point_lights) {
        pointLightSystem.render(frameInfo);
      }

      //Rendering UI
      ui.renderUI(commandBuffer, nileRenderer);

      nileRenderer.endSwapChainRenderPass(commandBuffer);
      nileRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(nileDevice.device());
}

void Terrain::loadGameObjects() {

  ProceduralTerrain proceduralTerrain{
    nileDevice,
    0,
    0,
    nullptr, 
    "../resources/images/heightMap.png"};
    

  std::shared_ptr<NileModel> mesh = proceduralTerrain.mesh;
  std::shared_ptr<NileTexture> simpleTexture =
      NileTexture::createTextureFromFile(nileDevice, "../resources/images/simple.png");
  auto& terrain = gameObjectManager.createGameObject();
  terrain.model = mesh;
  terrain.transform.translation = {ui.terrain_pos.x, -ui.terrain_pos.y, ui.terrain_pos.z};
  terrain.transform.scale = {1.5f, 1.f, 1.5f};
  terrain.transform.rotation = {ui.terrain_rot.x, ui.terrain_rot.y, ui.terrain_rot.z};
  // terrain.color  = {6.f, 1.f, 6.f};
  // terrain.diffuseMap = simpleTexture;
  obj_id = terrain.getId();

  std::shared_ptr<NileModel> floorModel = 
      NileModel::createModelFromFile(nileDevice, "resources/models/quad.obj");
  auto& floor = gameObjectManager.createGameObject();
  floor.model = floorModel;
  floor.transform.translation = {0.f, .25f, 0.f};
  floor.transform.scale = {3.f, 1.f, 3.f};
  // floor.color  = {6.f, 1.f, 6.f};
  
  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f}  //
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto& pointLight = gameObjectManager.makePointLight(0.2f);
    pointLight.color = lightColors[i];
    auto rotateLight = glm::rotate(
        glm::mat4(1.f),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.f, 0.f});
    pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
  }

  auto& pointLight = gameObjectManager.makePointLight(0.2f);
  pointLight.color = glm::vec3(1.f);
  pointLight.transform.translation = {0.f, -1.f, 0.f};
}
} // namespace nile
