#include "window.hpp"
#include "exception.hpp"

namespace core {
Window::Window(const uint32_t width, const uint32_t height, std::string title) {
  if (glfwInit() == GLFW_FALSE) {
    throw VulkanKraftException("failed to initialise GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  if (m_window =
          glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
      !m_window) {
    throw VulkanKraftException("failed to create window");
  }
}

Window::~Window() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}
} // namespace core