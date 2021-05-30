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
  glfwSetKeyCallback(m_window, _on_key_callback);
  glfwSetCursorPosCallback(m_window, _on_cursor_pos_callback);
}

Window::~Window() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Window::poll_events() { glfwPollEvents(); }

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

void Window::reset_keys() {
  for (auto [k, p] : m_pressed_keys) {
    m_previous_pressed_keys[k] = p;
  }
}

bool Window::key_is_pressed(int key) const {
  return m_pressed_keys.find(key) != m_pressed_keys.end() &&
         m_pressed_keys.at(key);
}

bool Window::key_just_pressed(int key) const {
  return key_is_pressed(key) &&
         (m_previous_pressed_keys.find(key) == m_previous_pressed_keys.end() ||
          !m_previous_pressed_keys.at(key));
}

void Window::_on_key_callback(GLFWwindow *window, int key, int scancode,
                              int action, int mods) {
  auto *win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  win->_on_key(key, scancode, action, mods);
}

void Window::_on_cursor_pos_callback(GLFWwindow *window, double x, double y) {
  auto *win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  win->_on_cursor_position(x, y);
}

void Window::_on_key(int key, int scancode, int action, int mods) {
  m_pressed_keys[key] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

void Window::_on_cursor_position(double x, double y) {
  m_mouse.screen_position.x = static_cast<float>(x);
  m_mouse.screen_position.y = static_cast<float>(y);
}
} // namespace core