#include "apps/app.hpp"

#include "framework/systems/lights/point_light_system.hpp"
#include "framework/systems/mirror/mirror_system.hpp"

namespace nile{
class Mirror : public App3D {

public:
  Mirror();
  ~Mirror() override;

  void loop() override;
  void start() override;

private:
  void loadGameObjects() override;
  NileGameObject::id_t target = 0;
};

Mirror::Mirror() { 
}

void Mirror::start() {
  nileRenderer.createOffScreen();
  loadGameObjects();
  loop();
}

Mirror::~Mirror() {}

void Mirror::loop() {
  RenderSystem3D objMirrored{
      nileDevice,
      nileRenderer.getOffScreenRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightMirrored{
      nileDevice,
      nileRenderer.getOffScreenRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  RenderSystem3D objSystem{
      nileDevice,
      nileRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightSystem{
      nileDevice,
      nileRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  MirrorSystem mirrorSystem{
      nileDevice,
      nileRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()
  };

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

      pointLightSystem.update(frameInfo, ubo);
      
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // final step of upate is updating the game objects buffer data
      // The render functions MUST not change a game objects transform data
      gameObjectManager.updateBuffer(frameIndex);

      // objSystem.updateSceneObject(target, frameInfo);

      // offscreen
      nileRenderer.beginOffScreenRenderPass(commandBuffer);
      objMirrored.renderGameObjects(frameInfo);
      pointLightMirrored.render(frameInfo);
      nileRenderer.endOffScreenRenderPass(commandBuffer);

      // render
      nileRenderer.beginSwapChainRenderPass(commandBuffer);

      // order here matters
      objSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);      
      mirrorSystem.renderMirrorPlane(frameInfo);

      //Rendering UI
      ui.renderUI(commandBuffer, nileRenderer);

      nileRenderer.endSwapChainRenderPass(commandBuffer);
      nileRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(nileDevice.device());
}

void Mirror::loadGameObjects() {

  std::shared_ptr<NileModel> plane = 
      NileModel::createModelFromFile(nileDevice, "resources/models/quad.obj");
  auto& tile = gameObjectManager.createGameObject();
  tile.model = plane;
  tile.transform.translation = {-.6f, .25f, 0.f};
  tile.transform.scale = {6.f, 1.f, 6.f};
  tile.transform.rotation.z += 3.16f;
  tile.transform.rotation.y += 3.f;
  tile.reflectionTexture = nileRenderer.nileOffScreen;
  tile.isMirror = true;

  std::shared_ptr<NileTexture> modelTexture =
        NileTexture::createTextureFromFile(nileDevice, "../resources/images/dragon.png");
  std::shared_ptr<NileModel> model = 
      NileModel::createModelFromFile(nileDevice, "resources/models/dragon.obj");
  auto& dragon = gameObjectManager.createGameObject();
  dragon.model = model;
  dragon.diffuseMap = modelTexture;
  dragon.transform.translation = {0.f, -1.25f, 0.f};
  dragon.transform.scale = {3.f, -1.f, 3.f};
  target = dragon.getId();
  
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
    pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -2.99f, 1.f, 1.f));
  }

  auto& pointLight = gameObjectManager.makePointLight(0.2f);
  pointLight.color = glm::vec3(1.f);
  pointLight.transform.translation = {0.f, -2.99f, 0.f};
  pointLight.transform.scale = glm::vec3(.3f);
}
} // namespace nile
