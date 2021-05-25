#include "window.hpp"
#include "exception.hpp"

namespace core {
void Window::on_framebuffer_resize(GLFWwindow *window, int width, int height) {
  auto *win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  win->m_fb_resize_cb(static_cast<uint32_t>(width),
                      static_cast<uint32_t>(height));
}

Window::Window(const uint32_t width, const uint32_t height, std::string title) {
  if (glfwInit() == GLFW_FALSE) {
    throw VulkanKraftException("failed to initialise GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  if (m_window =
          glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
      !m_window) {
    throw VulkanKraftException("failed to create window");
  }

  glfwSetWindowUserPointer(m_window, this);
  glfwSetFramebufferSizeCallback(m_window, on_framebuffer_resize);
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

vk::SurfaceKHR Window::create_vulkan_surface(vk::Instance &instance) const {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(instance.operator VkInstance(), m_window, nullptr,
                              &surface) != VK_SUCCESS) {
    throw VulkanKraftException("failed to create surface");
  }

  return surface;
}
} // namespace core