#pragma once

#include "../rendering/render_system.hpp"

namespace nile{
class MirrorSystem{
 public:
  MirrorSystem(
      NileDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout offscreenSetLayout);
  ~MirrorSystem();

  MirrorSystem(const MirrorSystem &) = delete;
  MirrorSystem &operator=(const MirrorSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo);
  void renderMirrorPlane(FrameInfo &frameInfo);

 private:
  void createPipeline(VkRenderPass renderPass);
  void createPipelineLayout(VkDescriptorSetLayout offscreenSetLayout);

  NileDevice &nileDevice;
  std::unique_ptr<NileDescriptorSetLayout> renderSystemLayout;
  std::unique_ptr<NilePipeline> nilePipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace nile
