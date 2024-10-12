#pragma once

#include "nile_buffer.hpp"
#include "nile_device.hpp"

#include "../systems/material/material_pack_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace nile{
class NileModel {
 public:
  struct Vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return position == other.position && color == other.color && normal == other.normal &&
             uv == other.uv;
    }
  };

  struct Vertex2D {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex2D &other) const {
      return position == other.position && color == other.color;
    }
  };

  struct Builder {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    void loadModel(const std::string &filepath);
  };

  NileModel(NileDevice &device, const NileModel::Builder &builder);
  NileModel(NileDevice &device, const NileModel::Builder &builder, std::shared_ptr<Material> material);
  NileModel(NileDevice &device, const NileModel::Builder &builder, std::shared_ptr<MaterialPack> texturePack);
  ~NileModel();

  NileModel(const NileModel &) = delete;
  NileModel &operator=(const NileModel &) = delete;

  static std::unique_ptr<NileModel> createModelFromFile(
      NileDevice &device, const std::string &filepath);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

  std::shared_ptr<Material> getMaterial() { return material; }
  std::shared_ptr<MaterialPack> getMaterialPack() { return texturePack; }

 private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createVertexBuffers(const std::vector<Vertex2D> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);

    NileDevice &nileDevice;
    std::shared_ptr<Material> material;
    std::shared_ptr<MaterialPack> texturePack;

  std::unique_ptr<NileBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<NileBuffer> indexBuffer;
  uint32_t indexCount;

};
}  // namespace nile
