#include "mirror_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

namespace nile{

struct MirrorPushConstants {
  glm::mat4 transform{1.f};
};

MirrorSystem::MirrorSystem(
    NileDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout offscreenSetLayout)
  : nileDevice{device} {
  createPipelineLayout(offscreenSetLayout);
  createPipeline(renderPass);
}

MirrorSystem::~MirrorSystem() {
  vkDestroyPipelineLayout(nileDevice.device(), pipelineLayout, nullptr);
}

void MirrorSystem::createPipelineLayout(VkDescriptorSetLayout offscreenSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(MirrorPushConstants);

  renderSystemLayout = 
      NileDescriptorSetLayout::Builder(nileDevice)
          .addBinding(
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
    offscreenSetLayout,
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

void MirrorSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  NilePipeline::defaultPipelineConfigInfo(pipelineConfig);

  // pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  nilePipeline = std::make_unique<NilePipeline>(
      nileDevice,
      "shaders/mirror.vert.spv",
      "shaders/mirror.frag.spv",
      pipelineConfig);
}

void MirrorSystem::renderMirrorPlane(FrameInfo& frameInfo) {
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
  std::unordered_map<NileGameObject*, VkDescriptorSet> cachedDesriptorSets;

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr || !obj.isMirror || obj.getIsHidden()) continue;

    // Check if the descriptor set is already cached
    auto it = cachedDesriptorSets.find(&obj);
    if (it != cachedDesriptorSets.end()) {
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
    } else {
      auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
      // auto diffuseMapInfo = obj.diffuseMap->getImageInfo();
      auto diffuseMapInfo =  obj.reflectionTexture->getImageInfo();
      VkDescriptorSet gameObjectDescriptorSet;
      NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
          .writeBuffer(0, &bufferInfo)
          .writeImage(1, &diffuseMapInfo)
          .build(gameObjectDescriptorSet);
      
      cachedDesriptorSets[&obj] = gameObjectDescriptorSet;

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

    MirrorPushConstants push{};
    push.transform = obj.transform.mat4();

    vkCmdPushConstants(
      frameInfo.commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(MirrorPushConstants),
      &push);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

}  // namespace nile
