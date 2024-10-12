#include "nile_model.hpp"

#include "nile_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>
//#include <type_traits>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace std {
template <>
struct hash<nile::NileModel::Vertex> {
  size_t operator()(nile::NileModel::Vertex const &vertex) const {
    size_t seed = 0;
    nile::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};
}  // namespace std

namespace nile{

NileModel::NileModel(NileDevice &device, const NileModel::Builder &builder) : nileDevice{device} {
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
}

NileModel::NileModel(NileDevice &device, const NileModel::Builder &builder, std::shared_ptr<Material> material) 
: nileDevice{device}, material{material} {
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
  if(material != nullptr) {
    material->create();
  }
}

NileModel::NileModel(NileDevice &device, const NileModel::Builder &builder, std::shared_ptr<MaterialPack> texturePack) 
: nileDevice{device}, texturePack{texturePack} {
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
  if(texturePack != nullptr) {
    texturePack->create();
  }
}

NileModel::~NileModel() {}

std::unique_ptr<NileModel> NileModel::createModelFromFile(
    NileDevice &device, const std::string &filepath) {
  Builder builder{};
  builder.loadModel(ENGINE_DIR + filepath);
  return std::make_unique<NileModel>(device, builder);
}

void NileModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  auto count = static_cast<uint32_t>(vertices.size());
  uint32_t size = sizeof(vertices[0]);

  NileBuffer stagingBuffer{
      nileDevice, 
      size, 
      count,  
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  VkDeviceSize bufferSize = sizeof(vertices[0]) * count;

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());
    vertexBuffer = std::make_unique<NileBuffer>(
      nileDevice,
      size,
      count,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vertexCount = count;
    if(vertexCount < 3) {
       throw std::runtime_error("Vertex count must be at least 3");
    }
    nileDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
  
}

void NileModel::createVertexBuffers(const std::vector<Vertex2D> &vertices) {
  auto count = static_cast<uint32_t>(vertices.size());
  uint32_t size = sizeof(vertices[0]);

  NileBuffer stagingBuffer{
      nileDevice, 
      size, 
      count,  
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  VkDeviceSize bufferSize = sizeof(vertices[0]) * count;

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());
    vertexBuffer = std::make_unique<NileBuffer>(
      nileDevice,
      size,
      count,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vertexCount = count;
    if(vertexCount < 3) {
       throw std::runtime_error("Vertex count must be at least 3");
    }
    nileDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
  
}

void NileModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
  auto count = static_cast<uint32_t>(indices.size());
  uint32_t size = sizeof(indices[0]);

  NileBuffer stagingBuffer{
      nileDevice, 
      size, 
      count,  
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  VkDeviceSize bufferSize = sizeof(indices[0]) * count;

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());
    // Function received a Index values
    indexBuffer = std::make_unique<NileBuffer>(
      nileDevice,
      size,
      count,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    indexCount = count;
    hasIndexBuffer = indexCount > 0;
    if(!hasIndexBuffer) {
      throw std::runtime_error("Index count must be at least 1");
    }
    nileDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void NileModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void NileModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
  }
}

std::vector<VkVertexInputBindingDescription> NileModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> NileModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
  attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
  attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
  attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

  return attributeDescriptions;
}

void NileModel::Builder::loadModel(const std::string &filepath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
    throw std::runtime_error(warn + err);
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};
  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};

      if (index.vertex_index >= 0) {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
      }

      if (index.normal_index >= 0) {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
        };
      }

      if (index.texcoord_index >= 0) {
        vertex.uv = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }
}

}  // namespace nile
