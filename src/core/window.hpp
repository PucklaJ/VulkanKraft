#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
class Window {
public:
  Window(const uint32_t width, const uint32_t height, std::string title);
  ~Window();

  inline bool should_close() const { return glfwWindowShouldClose(m_window); }
  inline void poll_events() const { glfwPollEvents(); }
  std::vector<const char *> get_required_vulkan_extensions() const;
  vk::SurfaceKHR create_vulkan_surface(vk::Instance &instance) const;
  inline std::pair<uint32_t, uint32_t> get_framebuffer_size() const {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    return std::make_pair(static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height));
  }

private:
  GLFWwindow *m_window;
};
} // namespace core