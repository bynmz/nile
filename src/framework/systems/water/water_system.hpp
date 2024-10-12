
#include "framework/core/nile_texture.hpp"
#include "framework/core/nile_frame_info.hpp"
#include "framework/core/nile_device.hpp"
#include "framework/core/nile_pipeline.hpp"
#include "framework/core/nile_game_object.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <string>
#include <memory>

namespace nile
{
class WaterSystem
{
public:
static constexpr float TILE_SIZE = 6.f;

WaterSystem(
    NileDevice& device,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout);
~WaterSystem();

// int getID();

glm::vec3 getPosition();
glm::vec3 getRotation();
glm::vec3 getScale();

std::vector<float> getVertices();

float getTileSize();
float getHeight();

std::shared_ptr<NileTexture> getDudvTexture();
std::shared_ptr<NileTexture> getNormalTexture();

void render(FrameInfo& frameInfo);
void renderMaps(FrameInfo& frameInfo);

private:

void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
void createPipeline(VkRenderPass renderPass);
VkDescriptorImageInfo imageDescriptor;


std::shared_ptr<NileTexture> dudvTexture;
std::shared_ptr<NileTexture> normalTexture;

NileDevice& device;
std::unique_ptr<NilePipeline> nilePipeline;
VkPipelineLayout pipelineLayout;

std::unique_ptr<NileDescriptorSetLayout> renderSystemLayout;
};
} // namespace nile

