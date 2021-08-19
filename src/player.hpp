#pragma once
#include "chunk/world.hpp"
#include "core/fps_timer.hpp"
#include "core/render_2d.hpp"
#include "core/resource_hodler.hpp"
#include "core/vulkan/render_call.hpp"
#include "core/window.hpp"
#include "physics/aabb.hpp"
#include "physics/moving_object.hpp"
#include "physics/server.hpp"
#include <glm/glm.hpp>

// The player class representing the character of the user and everything he can
// do
class Player : public physics::MovingObject {
public:
  // Give the player an initial position
  Player(const glm::vec3 &position, core::ResourceHodler &hodler,
         physics::Server &physics_server);

  // Returns the position of the eyes (m_position + eye_height)
  inline glm::vec3 get_eye_position() const {
    return position + glm::vec3(0.0f, eye_height, 0.0f);
  }
  // Returns the direction which the player is currently facing
  glm::vec3 get_look_direction() const;

  // Create a camera view matrix from the players look direction and position
  glm::mat4 create_view_matrix() const;

  // Update everything the player does (input, block placing/breaking, etc.)
  void update(core::Window &window, chunk::World &world);

  // returns the projection matrix used for the crosshair
  void render(const core::vulkan::RenderCall &render_call);

private:
  // The height of the player in blocks
  static constexpr float eye_height = 1.8f;
  // Scales the crosshair size
  static constexpr float crosshair_scale = 6.0f;
  static constexpr float aabb_width = 0.7f;
  static constexpr float aabb_depth = 0.7f;
  static constexpr float move_speed = 5.0f;
  static constexpr float jump_power = 5.0f;

  // Handles the controller dead zone. If the axis values absolute value is
  // below min, then the returned value will be 0 otherwise it will be the value
  static inline float _abs_dead_zone(const float value, const float min) {
    const auto abs{(value < 0.0f) * -value + (value >= 0.0f) * value};
    return (abs >= min) * value;
  }

  // ****** Update ******
  void _update_input(core::Window &window, bool &button_up, bool &button_down,
                     bool &button_place, bool &button_destroy,
                     glm::vec2 &move_direction, glm::vec2 &view);
  // ********************

  // The current rotation of the head (controlled by the mouse/gamepad)
  glm::vec2 m_rotation;

  // The last horizontal position of the mouse cursor
  float m_last_mouse_x;
  // The last vertical position of the mouse cursor
  float m_last_mouse_y;
  // Wether the left trigger has been pressed in the last frame
  bool m_last_left_trigger;
  // Wether the right trigger has been pressed in the last frame
  bool m_last_right_trigger;

  // Used to render the crosshair in the middle of the screen
  core::Render2D m_crosshair;
};