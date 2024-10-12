#include "nile_window.hpp"

// std
#include <stdexcept>

namespace nile{

NileWindow::NileWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
  initWindow();
}

NileWindow::~NileWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void NileWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void NileWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
}

void NileWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto nileWindow = reinterpret_cast<NileWindow *>(glfwGetWindowUserPointer(window));
  nileWindow->framebufferResized = true;
  nileWindow->width = width;
  nileWindow->height = height;
}

}  // namespace nile
