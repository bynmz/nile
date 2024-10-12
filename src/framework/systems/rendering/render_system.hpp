#pragma once

#include "framework/core/nile_camera.hpp"
#include "framework/core/nile_device.hpp"
#include "framework/core/nile_frame_info.hpp"
#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_pipeline.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <vector>

namespace nile{
class SimpleRenderSystem {
 private:
  NileDevice &nileDevice;
  virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  virtual void createPipeline(VkRenderPass renderPass);
  
 public:
  SimpleRenderSystem(
      NileDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  virtual ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

 protected:
  VkPipelineLayout pipelineLayout;
  std::unique_ptr<NilePipeline> nilePipeline;
  std::unique_ptr<NileDescriptorSetLayout> renderSystemLayout;
  virtual void renderGameObjects(FrameInfo &frameInfo);

};

class RenderSystem3D : public SimpleRenderSystem
{
private:
    NileDevice& device;

    void createPipeline(VkRenderPass renderPass) override;
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;

public:
    RenderSystem3D(
        NileDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~RenderSystem3D() override;

    RenderSystem3D(const RenderSystem3D &) = delete;
    RenderSystem3D &operator=(const RenderSystem3D &) = delete;
    void renderGameObjects(FrameInfo &frameInfo) override;
    void updateSceneObject(NileGameObject::id_t target, FrameInfo& frameInfo);
};

class RenderSystem2D : public SimpleRenderSystem
{
private:
    NileDevice& device;

    void createPipeline(VkRenderPass renderPass) override;
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;

public:
    RenderSystem2D(
        NileDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~RenderSystem2D() override;

    RenderSystem2D(const RenderSystem2D &) = delete;
    RenderSystem2D &operator=(const RenderSystem2D &) = delete;
    void renderGameObjects(FrameInfo &frameInfo) override;
};

}  // namespace nile
