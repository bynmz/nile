#include "app_3d.hpp"

#include "framework/systems/lights/point_light_system.hpp"

namespace nile{

App3D::App3D() {
  globalPool =
      NileDescriptorPool::Builder(nileDevice)
          .setMaxSets(NileSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, NileSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();
  
  // build frame descriptor pools
  framePools.resize(NileSwapChain::MAX_FRAMES_IN_FLIGHT);
  auto framePoolBuilder = NileDescriptorPool::Builder(nileDevice)
                                .setMaxSets(1000)
                                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
  for (int i = 0; i < framePools.size(); i++) {
    framePools[i] = framePoolBuilder.build();
  }         
    
  globalSetLayout =
      NileDescriptorSetLayout::Builder(nileDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
          .build();

  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<NileBuffer>(
        nileDevice,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    NileDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }
  std::cout << "Alignment: " << nileDevice.properties.limits.minUniformBufferOffsetAlignment << "\n";
  std::cout << "atom size: " << nileDevice.properties.limits.nonCoherentAtomSize << "\n";
}

void App3D::start() {
  loadGameObjects();
  loop();
}

App3D::~App3D() {}

void App3D::loop() {

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

    if (auto commandBuffer = nileRenderer.beginFrame()) {
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

      // render
      nileRenderer.beginSwapChainRenderPass(commandBuffer);

      // order here matters
      renderSystem3D.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      // Rendering UI
      ui.renderUI(commandBuffer, nileRenderer);

      nileRenderer.endSwapChainRenderPass(commandBuffer);
      nileRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(nileDevice.device());
}

void App3D::loadGameObjects() {
  std::shared_ptr<NileModel> nileModel = 
      NileModel::createModelFromFile(nileDevice, "resources/models/quad.obj");
  std::shared_ptr<NileTexture> marbleTexture =
      NileTexture::createTextureFromFile(nileDevice, "../resources/images/missing.png");
  auto& floor = gameObjectManager.createGameObject();
  floor.model = nileModel;
  floor.transform.translation = {0.f, .5f, 0.f};
  floor.transform.scale = {6.f, 1.f, 6.f};
  
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
  pointLight.color = {1.f, 0.f, 0.f};
  pointLight.transform.translation = {0.f, -1.f, 0.f};

}

}  // namespace nile
