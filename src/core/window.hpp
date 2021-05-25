#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
class Window {
public:
  static void on_framebuffer_resize(GLFWwindow *window, int width, int height);

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
  inline void
  set_on_resize(std::function<void(uint32_t width, uint32_t height)> callback) {
    m_fb_resize_cb = callback;
  }

private:
  GLFWwindow *m_window;
  std::function<void(uint32_t width, uint32_t height)> m_fb_resize_cb;
};
} // namespace core