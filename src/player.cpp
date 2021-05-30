#include "player.hpp"
#include <glm/gtx/transform.hpp>

Player::Player(const glm::vec3 &position)
    : m_position(position), m_rotation(0.0f, 0.0f), m_last_mouse_x(0.0f),
      m_last_mouse_y(0.0f) {}

glm::mat4 Player::create_view_matrix() const {
  const auto eye_position(m_position + glm::vec3(0.0f, eye_height, 0.0f));
  const auto look_direction(_get_look_direction());

  return glm::lookAt(eye_position, eye_position + look_direction,
                     glm::vec3(0.0f, 1.0f, 0.0f));
}

void Player::update(const core::FPSTimer &timer, core::Window &window) {
  // **** handle camera *****
  const auto cur_mouse_x = window.get_mouse().screen_position.x;
  const auto cur_mouse_y = window.get_mouse().screen_position.y;
  if (window.cursor_is_locked()) {
    const auto delta_mouse_x = cur_mouse_x - m_last_mouse_x;
    const auto delta_mouse_y = cur_mouse_y - m_last_mouse_y;

    m_rotation.x += -delta_mouse_x * glm::radians(0.1f);
    m_rotation.y += -delta_mouse_y * glm::radians(0.1f);
  }
  m_last_mouse_x = cur_mouse_x;
  m_last_mouse_y = cur_mouse_y;
  // **************************

  if (window.get_mouse().button_just_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    window.lock_cursor();
  }

  if (window.key_just_pressed(GLFW_KEY_ESCAPE)) {
    window.release_cursor();
  }

  // **** handle movement *****
  constexpr auto move_speed = 20.0f;
  const auto look_direction(_get_look_direction());
  const auto right_direction(
      glm::normalize(glm::cross(look_direction, glm::vec3(0.0f, 1.0f, 0.0f))));
  if (window.key_is_pressed(GLFW_KEY_W)) {
    m_position += look_direction * move_speed * timer.get_delta_time();
  } else if (window.key_is_pressed(GLFW_KEY_S)) {
    m_position += look_direction * -move_speed * timer.get_delta_time();
  }
  if (window.key_is_pressed(GLFW_KEY_D)) {
    m_position += right_direction * move_speed * timer.get_delta_time();
  } else if (window.key_is_pressed(GLFW_KEY_A)) {
    m_position += right_direction * -move_speed * timer.get_delta_time();
  }
  if (window.key_is_pressed(GLFW_KEY_I)) {
    m_position +=
        glm::vec3(0.0f, 1.0f, 0.0f) * move_speed * timer.get_delta_time();
  } else if (window.key_is_pressed(GLFW_KEY_K)) {
    m_position +=
        glm::vec3(0.0f, 1.0f, 0.0f) * -move_speed * timer.get_delta_time();
  }
  // **************************
}

glm::vec3 Player::_get_look_direction() const {
  glm::vec3 look_direction(0.0f, 0.0f, -1.0f);

  const glm::mat4 y_rot_mat(
      glm::rotate(m_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f)));
  look_direction = y_rot_mat * glm::vec4(look_direction, 1.0f);

  const glm::vec3 x_axis = y_rot_mat * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  look_direction =
      glm::rotate(m_rotation.y, x_axis) * glm::vec4(look_direction, 1.0f);
  return look_direction;
}