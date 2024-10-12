#pragma once

#include "nile_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace nile{

class NileSwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  NileSwapChain(NileDevice &deviceRef, VkExtent2D windowExtent);
  NileSwapChain(
      NileDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<NileSwapChain> previous);

   ~NileSwapChain();

  NileSwapChain(const NileSwapChain &) = delete;
  NileSwapChain &operator=(const NileSwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
  VkRenderPass getRenderPass() { return renderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
   VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

   VkImageView createImageView(VkImage image, VkFormat format);

  void createImageInfo(VkImageCreateInfo imageInfo, 
    VkFormat format, 
    uint32_t width, 
    uint32_t height, 
    VkImageUsageFlagBits ufb);

  bool compareSwapFormats(const NileSwapChain &swapChain) const {
    return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
           swapChain.swapChainImageFormat == swapChainImageFormat;
  }
 void init();

 private:
  void createSwapChain();
   void createImageViews();
  void createTextureImageView();
   void createDepthResources();
   void createRenderPass();
   void createFramebuffers();
  void createSyncObjects();

private:
  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkImageView imageView;
  
private:
  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  

  NileDevice &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<NileSwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}  // namespace nile
