#pragma once

#include "framework/core/nile_camera.hpp"
#include "framework/core/nile_device.hpp"
#include "framework/core/nile_frame_info.hpp"
#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace nile{
class PointLightSystem {
 public:
  PointLightSystem(
      NileDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~PointLightSystem();

  PointLightSystem(const PointLightSystem &) = delete;
  PointLightSystem &operator=(const PointLightSystem &) = delete;

  void update(FrameInfo &frameInfo, GlobalUbo &ubo);
  void render(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  NileDevice &nileDevice;

  std::unique_ptr<NilePipeline> nilePipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace nile
