#include "water_system.hpp"

// std
#include <map>

namespace nile
{ 
struct WaterPushConstants {
    glm::vec4 position{};
    glm::vec4 rotation{};
    glm::vec4 scale{};
};

WaterSystem::WaterSystem(
    NileDevice& device, 
    VkRenderPass renderPass,
    VkDescriptorSetLayout offscreenSetLayout
    )
    : device(device)
{
    createPipelineLayout(offscreenSetLayout);
    createPipeline(renderPass);
}

WaterSystem::~WaterSystem()
{
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void WaterSystem::createPipelineLayout(VkDescriptorSetLayout offscreenSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(WaterPushConstants);

  renderSystemLayout =
     NileDescriptorSetLayout::Builder(device)
         .addBinding(
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
         .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
         .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
         .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
         .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
         .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
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
  if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void WaterSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "Cannont create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    NilePipeline::defaultPipelineConfigInfo(pipelineConfig);

    // pipelineConfig.attributeDescriptions.clear();
    // pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    nilePipeline = std::make_unique<NilePipeline>(
        device,
        "shaders/water.vert.spv",
        "shaders/water.frag.spv",
        pipelineConfig);
}

void WaterSystem::render(FrameInfo& frameInfo) {
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

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.water == nullptr) continue;
        auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
        auto reflectionImageInfo = obj.reflectionTexture->getImageInfo();
        auto refractionImageInfo = obj.refractionTexture->getImageInfo();

        VkDescriptorSet gameObjectDescriptorSet;
        NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(4, &reflectionImageInfo)
            .writeImage(5, &refractionImageInfo)
            .build(gameObjectDescriptorSet);

        vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &gameObjectDescriptorSet,
        0,
        nullptr);

        WaterPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.f);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(WaterPushConstants),
            &push);

        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
        }
}

void WaterSystem::renderMaps(FrameInfo& frameInfo) {
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

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.water == nullptr) continue;
        auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
        auto normalImageInfo = obj.normalMap->getImageInfo();
        auto depthImageInfo = obj.depthMap->getImageInfo();
        auto dudvImageInfo = obj.dudvMap->getImageInfo();
        auto reflectionImageInfo = obj.reflectionTexture->getImageInfo();
        auto refractionImageInfo = obj.refractionTexture->getImageInfo();

        VkDescriptorSet gameObjectDescriptorSet;
        NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &dudvImageInfo)
            .writeImage(2, &normalImageInfo)
            .writeImage(3, &depthImageInfo)
            .writeImage(4, &reflectionImageInfo)
            .writeImage(5, &refractionImageInfo)
            .build(gameObjectDescriptorSet);

        vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
        1,  // set count
        &gameObjectDescriptorSet,
        0,
        nullptr);

        WaterPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.f);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(WaterPushConstants),
            &push);

        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
        }
    }
}