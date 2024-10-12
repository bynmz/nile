#pragma once

#include "nile_device.hpp"
#include "nile_swap_chain.hpp"
#include "nile_off_screen.hpp"
#include "nile_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>
#include <array>

namespace nile{
class NileRenderer {
 public:
  NileRenderer(NileWindow &window, NileDevice &device);
  ~NileRenderer();

  NileRenderer(const NileRenderer &) = delete;
  NileRenderer &operator=(const NileRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const { return nileSwapChain->getRenderPass(); }
  VkRenderPass getOffScreenRenderPass() const { return nileOffScreen->getRenderPass(); }
  VkDescriptorImageInfo getOffScreenImageInfo() const { return nileOffScreen->getImageInfo(); }

  float getAspectRatio() const { return nileSwapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }
  size_t getImageCount() const {return nileSwapChain->imageCount(); }

  std::array<VkClearValue, 2> clearValues{};
  
  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  //void beginOffScreenRendering();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void beginOffScreenRenderPass(VkCommandBuffer commandBuffer);
  void endOffScreenRenderPass(VkCommandBuffer commandBuffer);
  VkImageView createImageView(const VkImage &textureImage, const VkFormat &format ) { return nileSwapChain->createImageView(textureImage, format); }
  void createOffScreen();
  
  std::shared_ptr<NileOffScreen> nileOffScreen;
  
 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  NileWindow &nileWindow;
  NileDevice &nileDevice;
  std::unique_ptr<NileSwapChain> nileSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};
}  // namespace nile
