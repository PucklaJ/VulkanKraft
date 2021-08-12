#pragma once
#include "chunk/world.hpp"
#include "core/fps_timer.hpp"
#include "core/window.hpp"
#include <glm/glm.hpp>

class Player {
public:
  Player(const glm::vec3 &position);

  inline void set_position(const glm::vec3 &pos) { m_position = pos; }
  inline void set_position(const float x, const float y, const float z) {
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
  }
  inline void set_height(const float y) { m_position.y = y; }
  inline const glm::vec3 &get_position() const { return m_position; }
  inline glm::vec3 get_look_direction() const { return _get_look_direction(); }

  glm::mat4 create_view_matrix() const;

  void update(const core::FPSTimer &timer, core::Window &window,
              chunk::World &world);

private:
  static constexpr float eye_height = 1.8f;

  glm::vec3 _get_look_direction() const;
  glm::vec3 _get_eye_position() const;

  glm::vec3 m_position;
  glm::vec2 m_rotation;

  float m_last_mouse_x;
  float m_last_mouse_y;
};