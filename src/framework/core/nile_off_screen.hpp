#pragma once

#include "nile_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

// Offscreen frame buffer properties
#define FB_DIM 512
#define FB_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM

namespace nile
{
class NileOffScreen {
public:

NileOffScreen(NileDevice &device);

~NileOffScreen();

NileOffScreen(const NileOffScreen &) = delete;
NileOffScreen &operator=(const NileOffScreen &) = delete;

VkRenderPass getRenderPass() const {return offscreenPass.renderPass;}
VkDescriptorImageInfo getImageInfo() const { return offscreenPass.descriptor; }
VkImageView createImageView(VkImage image, VkFormat format);

// Framebuffer for offscreen rendering
struct FrameBufferAttachment {
    VkImage image;
    VkDeviceMemory mem;
    VkBuffer staging;
    VkImageView view;
};

struct OffscreenPass {
    uint32_t width, height;
    VkFramebuffer frameBuffer;
    FrameBufferAttachment color, depth;
    VkRenderPass renderPass;
    VkSampler sampler;
    VkDescriptorImageInfo descriptor;
} offscreenPass{};
 
 void init();
 void transitionLayout(VkCommandBuffer commandBuffer, uint32_t t);

private:
 void createImage();
 void createImageView();
 void createRenderPass();
 void createDepthResources();
 void createFramebuffers();
 void createSampler();
 void updateDescriptor();

VkFormat findDepthFormat();

VkImageView imageView;
NileDevice& device;

};
} // namespace nile
