#pragma once

#include "nile_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace nile{

class NileDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(NileDevice &nileDevice) : nileDevice{nileDevice} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<NileDescriptorSetLayout> build() const;

   private:
    NileDevice &nileDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  NileDescriptorSetLayout(
      NileDevice &nileDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~NileDescriptorSetLayout();
  NileDescriptorSetLayout(const NileDescriptorSetLayout &) = delete;
  NileDescriptorSetLayout &operator=(const NileDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  NileDevice &nileDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class NileDescriptorWriter;
};

class NileDescriptorPool {
 public:
  class Builder {
   public:
    Builder(NileDevice &nileDevice) : nileDevice{nileDevice} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<NileDescriptorPool> build() const;

   private:
    NileDevice &nileDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  NileDescriptorPool(
      NileDevice &nileDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~NileDescriptorPool();
  NileDescriptorPool(const NileDescriptorPool &) = delete;
  NileDescriptorPool &operator=(const NileDescriptorPool &) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  NileDevice &nileDevice;
  VkDescriptorPool descriptorPool;

  friend class NileDescriptorWriter;
};

class NileDescriptorWriter {
 public:
  NileDescriptorWriter(NileDescriptorSetLayout &setLayout, NileDescriptorPool &pool);

  NileDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  NileDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  NileDescriptorSetLayout &setLayout;
  NileDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

}  // namespace nile
