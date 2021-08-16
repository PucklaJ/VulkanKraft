#pragma once
#include "chunk/world.hpp"
#include "core/fps_timer.hpp"
#include "core/render_2d.hpp"
#include "core/resource_hodler.hpp"
#include "core/vulkan/render_call.hpp"
#include "core/window.hpp"
#include <glm/glm.hpp>

// The player class representing the character of the user and everything he can
// do
class Player {
public:
  // Give the player an initial position
  Player(const glm::vec3 &position, core::ResourceHodler &hodler);

  inline void set_position(const glm::vec3 &pos) { m_position = pos; }
  inline void set_position(const float x, const float y, const float z) {
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
  }
  // Only set the y component of the position
  inline void set_height(const float y) { m_position.y = y; }
  inline const glm::vec3 &get_position() const { return m_position; }
  inline glm::vec3 get_eye_position() const { return _get_eye_position(); }
  inline glm::vec3 get_look_direction() const { return _get_look_direction(); }

  // Create a camera view matrix from the players look direction and position
  glm::mat4 create_view_matrix() const;

  // Update everything the player does (input, block placing/breaking, etc.)
  void update(const core::FPSTimer &timer, core::Window &window,
              chunk::World &world);
  // returns the projection matrix used for the crosshair
  void render(const core::vulkan::RenderCall &render_call);

private:
  // The height of the player in blocks
  static constexpr float eye_height = 1.8f;
  // Scales the crosshair size
  static constexpr float crosshair_scale = 6.0f;

  static inline float _abs_dead_zone(const float value, const float min) {
    const auto abs{(value < 0.0f) * -value + (value >= 0.0f) * value};
    return (abs >= min) * value;
  }

  // Returns the direction which the player is currently facing
  glm::vec3 _get_look_direction() const;
  // Returns the position of the eyes (m_position + eye_height)
  glm::vec3 _get_eye_position() const;

  // The position of the feet of the player
  glm::vec3 m_position;
  // The current rotation of the head (controlled by the mouse/gamepad)
  glm::vec2 m_rotation;

  // The last horizontal position of the mouse cursor
  float m_last_mouse_x;
  // The last vertical position of the mouse cursor
  float m_last_mouse_y;
  bool m_last_left_trigger;
  bool m_last_right_trigger;

  core::Render2D m_crosshair;
};