#include "render_system.hpp"

namespace nile{

struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

SimpleRenderSystem::SimpleRenderSystem(
    NileDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : nileDevice{device} 
{
  // createPipelineLayout(globalSetLayout);
  // createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(nileDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  renderSystemLayout =
      NileDescriptorSetLayout::Builder(nileDevice)
          .addBinding(
              0,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
    globalSetLayout, 
    renderSystemLayout->getDescriptorSetLayout()};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(nileDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  NilePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  nilePipeline = std::make_unique<NilePipeline>(
      nileDevice,
      "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv",
      pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
  nilePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      1,
      &frameInfo.globalDescriptorSet,
      0,
      nullptr);

  // Create a map to cache descriptor sets for each game object
  std::unordered_map<NileGameObject*, VkDescriptorSet> cachedDescriptorSets;

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr || obj.getIsHidden() ) continue;
    
    // Check if the descriptor set is already cached
    auto it = cachedDescriptorSets.find(&obj);
    if (it != cachedDescriptorSets.end()) {
      // If cached, bind the cached descriptor set
      vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &it->second,
        0,
        nullptr);
    }  else {
      auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
      auto diffuseMapInfo = obj.diffuseMap->getImageInfo();
      VkDescriptorSet gameObjectDescriptorSet;
      NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
          .writeBuffer(0, &bufferInfo)
          .writeImage(1, &diffuseMapInfo)
          .build(gameObjectDescriptorSet);

      cachedDescriptorSets[&obj] = gameObjectDescriptorSet;

      // Bind the newly created descriptor set
      vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &gameObjectDescriptorSet,
        0,
        nullptr);
    }

    SimplePushConstantData push{};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

struct PushConstantData {
  glm::mat2 transform{1.f};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

RenderSystem2D::RenderSystem2D(
  NileDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
: SimpleRenderSystem(device, renderPass, globalSetLayout), device{device}
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

RenderSystem2D::~RenderSystem2D(){}

void RenderSystem2D::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstantData);

  renderSystemLayout =
      NileDescriptorSetLayout::Builder(device)
          .addBinding(
              0,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
    globalSetLayout, 
    renderSystemLayout->getDescriptorSetLayout()};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void RenderSystem2D::createPipeline(VkRenderPass renderPass)
{
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  NilePipeline::defaultPipelineConfigInfo(pipelineConfig);
  // pipelineConfig.attributeDescriptions.clear();
  // pipelineConfig.bindingDescriptions.clear();
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;

    nilePipeline = std::make_unique<NilePipeline>(
        device,
        "shaders/simple_shader_2d.vert.spv",
        "shaders/simple_shader_2d.frag.spv",
        pipelineConfig);
}


void RenderSystem2D::renderGameObjects(FrameInfo& frameInfo) {
  nilePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      1,
      &frameInfo.globalDescriptorSet,
      0,
      nullptr);

  // Create a map to cache descriptor sets for each game object
  std::unordered_map<NileGameObject*, VkDescriptorSet> cachedDescriptorSets;

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr || obj.getIsHidden() || obj.Destroyed ) continue;
    
    // Check if the descriptor set is already cached
    auto it = cachedDescriptorSets.find(&obj);
    if (it != cachedDescriptorSets.end()) {
      // If cached, bind the cached descriptor set
      vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &it->second,
        0,
        nullptr);
    }  else {
      auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
      auto diffuseMapInfo = obj.diffuseMap->getImageInfo();
      VkDescriptorSet gameObjectDescriptorSet;
      NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
          .writeBuffer(0, &bufferInfo)
          .writeImage(1, &diffuseMapInfo)
          .build(gameObjectDescriptorSet);

      cachedDescriptorSets[&obj] = gameObjectDescriptorSet;

      // Bind the newly created descriptor set
      vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &gameObjectDescriptorSet,
        0,
        nullptr);
    }

    PushConstantData push{};
    push.offset = obj.transform2d.translation;
    push.color = obj.color;
    push.transform = obj.transform2d.mat2();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(PushConstantData),
        &push);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

// struct SimplePushConstantData {
//   glm::mat4 modelMatrix{1.f};
//   glm::mat4 normalMatrix{1.f};
// };

RenderSystem3D::RenderSystem3D(
    NileDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
: SimpleRenderSystem(device, renderPass, globalSetLayout), device{device} 
{
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

RenderSystem3D::~RenderSystem3D() {}

void RenderSystem3D::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  renderSystemLayout =
      NileDescriptorSetLayout::Builder(device)
          .addBinding(
              0,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
    globalSetLayout, 
    renderSystemLayout->getDescriptorSetLayout()};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void RenderSystem3D::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  NilePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  // pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

  nilePipeline = std::make_unique<NilePipeline>(
      device,
      "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv",
      pipelineConfig);
}

void RenderSystem3D::updateSceneObject(NileGameObject::id_t target, FrameInfo& frameInfo) {
    auto& obj = frameInfo.gameObjects.at(target);

    auto rotateObj = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, {0.f, -1.f, 0.f});

    // update object position
    obj.transform.translation = glm::vec3(rotateObj * glm::vec4(obj.transform.translation, 1.f));

}

void RenderSystem3D::renderGameObjects(FrameInfo& frameInfo) {
  nilePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      1,
      &frameInfo.globalDescriptorSet,
      0,
      nullptr);

  // Create a map to cache descriptor sets for each game object
  std::unordered_map<NileGameObject*, VkDescriptorSet> cachedDescriptorSets;

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr || obj.isMirror || obj.getIsHidden() ) continue;
    
    // Check if the descriptor set is already cached
    auto it = cachedDescriptorSets.find(&obj);
    if (it != cachedDescriptorSets.end()) {
      // If cached, bind the cached descriptor set
      vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &it->second,
        0,
        nullptr);
    }  else {
      auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
      auto diffuseMapInfo = obj.diffuseMap->getImageInfo();
      VkDescriptorSet gameObjectDescriptorSet;
      NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
          .writeBuffer(0, &bufferInfo)
          .writeImage(1, &diffuseMapInfo)
          .build(gameObjectDescriptorSet);

      cachedDescriptorSets[&obj] = gameObjectDescriptorSet;

      // Bind the newly created descriptor set
      vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &gameObjectDescriptorSet,
        0,
        nullptr);
    }

    SimplePushConstantData push{};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

}  // namespace nile
