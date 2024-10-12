#pragma once

#include "framework/core/nile_descriptors.hpp"
#include "framework/core/nile_device.hpp"
#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_renderer.hpp"
#include "framework/core/nile_window.hpp"
#include "framework/core/nile_buffer.hpp"
#include "framework/core/nile_camera.hpp"
#include "framework/core/nile_model.hpp"
#include "framework/ui/simple_ui.hpp"
#include "framework/input/keyboard.hpp"
#include "framework/systems/rendering/render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>

namespace nile{
class App2D {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  App2D();
  virtual ~App2D();

  App2D(const App2D &) = delete;
  App2D &operator=(const App2D &) = delete;

  static constexpr int MAX_FRAMES = NileSwapChain::MAX_FRAMES_IN_FLIGHT;

  NileWindow nileWindow{WIDTH, HEIGHT, "Nile Engine 2D"};
  NileDevice nileDevice{nileWindow};
  NileRenderer nileRenderer{nileWindow, nileDevice};
  
  // note: order of declaration matters
  std::unique_ptr<NileDescriptorPool> globalPool{};
  std::unique_ptr<NileDescriptorSetLayout> globalSetLayout{};
  std::vector<std::unique_ptr<NileDescriptorPool>> framePools;
  
  std::vector<std::unique_ptr<NileBuffer>> uboBuffers{MAX_FRAMES};
  std::vector<VkDescriptorSet> globalDescriptorSets{MAX_FRAMES};
  
  NileGameObjectManager gameObjectManager{nileDevice};
  

  // RenderSystem2D renderSystem2D{
  //   nileDevice,
  //   nileRenderer.getSwapChainRenderPass(),
  //   globalSetLayout->getDescriptorSetLayout()
  // };
  SimpleUI ui{
    nileDevice,
    nileWindow.getGLFWwindow(),
    nileRenderer.getSwapChainRenderPass(),
    nileRenderer.getImageCount()};
  NileCamera camera{};
  KeyboardMovementController playerController{};

  virtual void start();

  private:
  virtual void loadGameObjects();
  virtual void loop();

};
}  // namespace nile
