#pragma once

#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_window.hpp"
#include "framework/core/nile_frame_info.hpp"

namespace nile{
class KeyboardMovementController {
 public:
  struct KeyMappings {
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_E;
    int moveDown = GLFW_KEY_Q;
    int lookLeft = GLFW_KEY_LEFT;
    int lookRight = GLFW_KEY_RIGHT;
    int lookUp = GLFW_KEY_UP;
    int lookDown = GLFW_KEY_DOWN;
  };

  void moveInPlaneXYZ(GLFWwindow* window, float dt, NileGameObject& gameObject);
  
  void moveInPlaneXY(GLFWwindow* window, float dt, NileGameObject& gameObject);

  KeyMappings keys{};
  float moveSpeed{3.f};
  float lookSpeed{1.5f};
};
}  // namespace nile