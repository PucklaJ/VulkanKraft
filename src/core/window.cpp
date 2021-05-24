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

std::vector<const char *> Window::get_required_vulkan_extensions() const {
  uint32_t glfw_count;
  const auto **glfw_ext = glfwGetRequiredInstanceExtensions(&glfw_count);
  if (glfw_count == 0 || !glfw_ext) {
    throw VulkanKraftException(
        "failed to get required vulkan extensions from GLFW");
  }

  return std::vector(glfw_ext, glfw_ext + glfw_count);
}
} // namespace core