#pragma once

#include "nile_device.hpp"

// libs
#include <vulkan/vulkan.h>

// std
#include <memory>
#include <string>

namespace nile{
class NileTexture {
public:
NileTexture(NileDevice& device, const std::string &textureFilepath);
NileTexture(
    NileDevice& device, 
    VkFormat format,
    VkExtent3D extent,
    VkImageUsageFlags usage,
    VkSampleCountFlagBits sampleCount);
~NileTexture();

// delete copy constructors
NileTexture(const NileTexture &) = delete;
NileTexture &operator=(const NileTexture &) = delete;

VkImageView imageView() const { return mTextureImageView; }
VkSampler sampler() const { return mTextureSampler; }
VkImage getImage() const { return mTextureImage; }
VkImageView getImageView() const { return mTextureImageView; }
VkDescriptorImageInfo getImageInfo() const { return mDescriptor; }
VkImageLayout getImageLayout() const { return mTextureLayout; }
VkExtent3D getExtent() const {return mExtent;}
VkFormat getFormat() const { return mFormat; }

int getTextureWidth() {return texWidth;}
int getTextureHeight() {return texHeight;}

void updateDescriptor();
void transitionLayout(
    VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

static std::unique_ptr<NileTexture> createTextureFromFile(
    NileDevice &device, const std::string &filepath);

void destroy() {NileTexture::~NileTexture();}

private:
void createTextureImage(const std::string &filepath);
void createTextureImageView(VkImageViewType viewType);
void createTextureSampler();

VkDescriptorImageInfo mDescriptor{};

NileDevice& mDevice;
VkImage mTextureImage = nullptr;
VkDeviceMemory mTextureImageMemory = nullptr;
VkImageView mTextureImageView = nullptr;
VkSampler mTextureSampler = nullptr;
VkFormat mFormat;
VkImageLayout mTextureLayout;
uint32_t mMipLevels{1};
uint32_t mLayerCount{1};
VkExtent3D mExtent{};

int texWidth, texHeight, texChannels;

};
}  // namespace nile 