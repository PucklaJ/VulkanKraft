#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace core {
class Window {
public:
  Window(const uint32_t width, const uint32_t height, std::string title);
  ~Window();

  inline bool should_close() const { return glfwWindowShouldClose(m_window); }
  inline void poll_events() const { glfwPollEvents(); }

private:
  GLFWwindow *m_window;
};
} // namespace core