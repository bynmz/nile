#include "app_2d.hpp"
#include "framework/systems/lights/point_light_system.hpp"

namespace nile{

App2D::App2D() {
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

App2D::~App2D() {}

void App2D::start() {
  loadGameObjects();
  loop();
}

void App2D::loop() {

  RenderSystem2D renderSystem2D{
    nileDevice,
    nileRenderer.getSwapChainRenderPass(),
    globalSetLayout->getDescriptorSetLayout()
  };
  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!nileWindow.shouldClose()) {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    //Start the Dear ImGui frame 
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

      gameObjectManager.updateBuffer(frameIndex);
      // render
      nileRenderer.beginSwapChainRenderPass(commandBuffer);
     
      // order here matters
      renderSystem2D.renderGameObjects(frameInfo);

      // Rendering UI
      ui.renderUI(commandBuffer, nileRenderer);

      nileRenderer.endSwapChainRenderPass(commandBuffer);
      nileRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(nileDevice.device());
}

void App2D::loadGameObjects() {

  auto& viewerObject = gameObjectManager.createGameObject();
  viewerObject.transform.translation.z = -2.5f;

  std::shared_ptr<NileModel> nileModel = 
      NileModel::createModelFromFile(nileDevice, "resources/models/quad.obj");
  std::shared_ptr<NileTexture> marbleTexture =
      NileTexture::createTextureFromFile(nileDevice, "../resources/images/missing.png");
  auto& floor = gameObjectManager.createGameObject();
  floor.model = nileModel;
  floor.transform.translation = {0.f, .5f, 0.f};
  floor.transform.scale = {1.f, 0.2f, 1.f};

  NileModel::Builder meshBuilder{};
  meshBuilder.vertices = {
      // base (red)
      {{-.5f, -.5f, -.5f}, {1.f, 0.f, 0.f}},
      {{.5f, -.5f, -.5f}, {0.f, 1.f, 0.f}},
      {{.5f, .5f, -.5f}, {0.f, 0.f, 1.f}},
      {{-.5f, .5f, -.5f}, {0.f, 1.f, 0.f}},
  };
  meshBuilder.indices = {0, 1, 2, 0};
  auto tile2 = std::make_shared<NileModel>(nileDevice, meshBuilder);
  auto& square2 = gameObjectManager.createGameObject();
  square2.model = tile2;
  square2.color = glm::vec3(1.f);
}

}  // namespace nile
