#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
// Represents the window on the screen. Also handles things like input and so on
class Window {
public:
  // Stores data of the mouse position and mouse buttons
  class Mouse {
  public:
    friend class Window;

    Mouse() : screen_position(0.0f, 0.0f) {}

    // The current position of the mouse in screen coordinates
    glm::vec2 screen_position;

    // Wether the given GLFW mouse button is currently pressed
    bool button_is_pressed(int button) const;
    // Wether the given GLFW mouse button is pressed in this frame but hasn't
    // been in the last frame
    bool button_just_pressed(int button) const;
    // Wether the given GLFW mouse button was pressed in the last frame
    bool button_was_pressed(int button) const;

  private:
    // A callback for GLFW
    void _on_mouse_button(int button, int action, int mods);

    // Stores the current states of the mouse buttons
    std::map<int, bool> m_pressed_buttons;
    // Stores the states of the mouse buttons of the last frame
    std::map<int, bool> m_previous_pressed_buttons;
  };

  // A callback for GLFW
  static void on_framebuffer_resize(GLFWwindow *window, int width, int height);
  static void on_joystick_event(int jid, int event);

  // Initalise the window given the width and height in pixels as well as a
  // window title
  Window(const uint32_t width, const uint32_t height, std::string title);
  ~Window();

  // Wether the window has been closed by the user and therefore should be
  // destroyed
  inline bool should_close() const { return glfwWindowShouldClose(m_window); }
  // Handle the window events
  void poll_events();
  // Returns all extensions required by the window to make vulkan able to be
  // used with this window
  std::vector<const char *> get_required_vulkan_extensions() const;
  // Creates a surface for this window
  vk::SurfaceKHR create_vulkan_surface(vk::Instance &instance) const;
  // Returns the current size of the frame buffer (window size) in pixels
  inline std::pair<uint32_t, uint32_t> get_framebuffer_size() const {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    return std::make_pair(static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height));
  }
  // Add a framebuffer resize callback
  inline void
  set_on_resize(std::function<void(uint32_t width, uint32_t height)> callback) {
    m_fb_resize_callbacks.emplace_back(callback);
  }
  inline const Mouse &get_mouse() const { return m_mouse; }

  // Tell the window that the input for the frame has ended and set all inputs
  // of the last frame to the current ones
  void reset_keys();
  // Wether the given GLFW key is currently pressed
  bool key_is_pressed(int key) const;
  // Wether the given GLFW key is currently pressed and hasn't been in the last
  // frame
  bool key_just_pressed(int key) const;
  // Lock the cursor in the window
  void lock_cursor();
  // Release the cursor from the window
  void release_cursor();
  // Wether the cursor is currently locked in the window
  bool cursor_is_locked();
  // Sets the window into fullscreen mode or goes out of fullscreen mode
  void toggle_fullscreen();

  // ***** Gamepad ******
  bool is_gamepad_connected();

  bool gamepad_button_is_pressed(int button) const;

  bool gamepad_button_just_pressed(int button) const;

  std::optional<float> get_gamepad_axis_value(int axis) const;
  // ********************

private:
  // A callback used for GLFW
  static void _on_key_callback(GLFWwindow *window, int key, int scancode,
                               int action, int mods);
  // A callback used for GLFW
  static void _on_cursor_pos_callback(GLFWwindow *window, double x, double y);
  // A callback used for GLFW
  static void _on_mouse_button_callback(GLFWwindow *window, int button,
                                        int action, int mods);

  void _on_key(int key, int scancode, int action, int mods);
  void _on_cursor_position(double x, double y);

  GLFWwindow *m_window;
  std::vector<std::function<void(uint32_t width, uint32_t height)>>
      m_fb_resize_callbacks;
  // The state of the keys in the current frame
  std::map<int, bool> m_pressed_keys;
  // The state of the keys in the last frame
  std::map<int, bool> m_previous_pressed_keys;
  std::map<int, bool> m_pressed_gamepad_buttons;
  std::map<int, bool> m_previous_pressed_gamepad_buttons;
  std::map<int, float> m_gamepad_axes;
  Mouse m_mouse;

  int m_previous_width;
  int m_previous_height;
  int m_previous_x;
  int m_previous_y;
  bool m_is_fullscreen;

  int m_joystick1;
};
} // namespace core