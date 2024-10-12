#pragma once

#include "framework/core/nile_descriptors.hpp"
#include "framework/core/nile_device.hpp"
#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_renderer.hpp"
#include "framework/core/nile_window.hpp"
#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_buffer.hpp"
#include "framework/core/nile_camera.hpp"
#include "framework/core/nile_model.hpp"
#include "framework/input/keyboard.hpp"
#include "framework/ui/simple_ui.hpp"
#include "framework/systems/rendering/render_system.hpp"

// std
#include <memory>
#include <vector>
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace nile{
class App3D {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  App3D();
  virtual ~App3D();

  App3D(const App3D &) = delete;
  App3D &operator=(const App3D &) = delete;

  static constexpr int MAX_FRAMES = NileSwapChain::MAX_FRAMES_IN_FLIGHT;

  NileWindow nileWindow{WIDTH, HEIGHT, "Nile Engine 3D"};
  NileDevice nileDevice{nileWindow};
  NileRenderer nileRenderer{nileWindow, nileDevice};

  // note: order of declarations matters
  std::unique_ptr<NileDescriptorPool> globalPool{};

  std::unique_ptr<NileDescriptorSetLayout> globalSetLayout{};
  std::vector<std::unique_ptr<NileDescriptorPool>> framePools;
  NileGameObjectManager gameObjectManager{nileDevice};

  std::vector<std::unique_ptr<NileBuffer>> uboBuffers{MAX_FRAMES};
  std::vector<VkDescriptorSet> globalDescriptorSets{MAX_FRAMES};

  SimpleUI ui{
    nileDevice,
    nileWindow.getGLFWwindow(),
    nileRenderer.getSwapChainRenderPass(),
    nileRenderer.getImageCount()
  };

  NileCamera camera{};
  KeyboardMovementController cameraController{};

  virtual void start();

  private:
  virtual void loadGameObjects();
  virtual void loop();
};
}  // namespace nile
