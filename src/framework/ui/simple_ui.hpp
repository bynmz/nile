#pragma once

#include "framework/core/nile_device.hpp"
#include "framework/core/nile_swap_chain.hpp"

#include "framework/core/nile_renderer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace nile{
class SimpleUI
{
private:
void loadFonts();

NileDevice& mDevice; 
VkRenderPass mRenderPass;
size_t mImageCount; 
GLFWwindow* mWindow;

// Data
VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
QueueFamilyIndices indices = mDevice.findPhysicalQueueFamilies();
VkQueue mQueue = mDevice.graphicsQueue();
VkResult err;
VkDescriptorPool imguiPool;
ImGuiIO& io;

// UI State
bool show_demo_window = false;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


public:
SimpleUI(NileDevice& device,
        GLFWwindow* window,
        VkRenderPass renderPass, 
        size_t imageCount);
~SimpleUI();

float getFrameRate() { return io.Framerate; }
ImVec2 getMousePos() { return io.MousePos; }
float getDeltaTime() { return io.DeltaTime; }

// Scene configs
bool enable_point_lights = false;
bool set_show_model = true;

glm::vec3 terrain_pos = {4.6f, 1.9f, 4.6f};
glm::vec3 terrain_rot = glm::vec3(0.f);

glm::vec3 p_lights_pos = glm::vec3(0.f);

void init();
// delete copy constructors
SimpleUI(const SimpleUI &) = delete;
SimpleUI &operator=(const SimpleUI &) = delete;

void startUI();
void renderUI(VkCommandBuffer commandBuffer, NileRenderer &renderer);

static void check_vk_result(VkResult err);

};
}