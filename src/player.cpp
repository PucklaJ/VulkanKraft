#include "player.hpp"
#include "core/exception.hpp"
#include "core/log.hpp"
#include <glm/gtx/transform.hpp>

Player::Player(const glm::vec3 &position, core::ResourceHodler &hodler)
    : m_position(position), m_rotation(0.0f, 0.0f), m_last_mouse_x(0.0f),
      m_last_mouse_y(0.0f),
      m_crosshair(
          hodler.get_texture(core::ResourceHodler::crosshair_texture_name)) {}

glm::mat4 Player::create_view_matrix() const {
  const auto eye_position(_get_eye_position());
  const auto look_direction(_get_look_direction());

  return glm::lookAt(eye_position, eye_position + look_direction,
                     glm::vec3(0.0f, 1.0f, 0.0f));
}

void Player::update(const core::FPSTimer &timer, core::Window &window,
                    chunk::World &world) {
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

  // **** handle movement *****
  constexpr auto move_speed = 20.0f;
  const auto look_direction(_get_look_direction());
  const auto forward(
      glm::normalize(glm::vec3(look_direction.x, 0.0f, look_direction.z)));
  const auto right_direction(
      glm::normalize(glm::cross(look_direction, glm::vec3(0.0f, 1.0f, 0.0f))));
  if (window.key_is_pressed(GLFW_KEY_W)) {
    m_position += forward * move_speed * timer.get_delta_time();
  } else if (window.key_is_pressed(GLFW_KEY_S)) {
    m_position += forward * -move_speed * timer.get_delta_time();
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

  if (window.cursor_is_locked() || window.key_just_pressed(GLFW_KEY_N)) {
    if ((window.key_is_pressed(GLFW_KEY_LEFT_SHIFT) &&
         window.get_mouse().button_is_pressed(GLFW_MOUSE_BUTTON_LEFT)) ||
        window.get_mouse().button_just_pressed(GLFW_MOUSE_BUTTON_LEFT) ||
        window.key_just_pressed(GLFW_KEY_N)) {
      const core::math::Ray ray{_get_eye_position(), look_direction};
      core::math::Ray::Face face;
      const auto _block(world.raycast_block(ray, face));
      if (_block) {
        const auto block = *_block;

        try {
          world.destroy_block(block);
        } catch (const core::VulkanKraftException &e) {
          core::Log::warning(std::string("failed to destroy block: ") +
                             e.what());
        }
      }
    } else if (window.get_mouse().button_just_pressed(
                   GLFW_MOUSE_BUTTON_RIGHT)) {
      const core::math::Ray ray{_get_eye_position(), look_direction};
      core::math::Ray::Face face;
      const auto _block(world.raycast_block(ray, face));
      if (_block) {
        auto block = *_block;

        switch (face) {
        case core::math::Ray::Face::FRONT:
          block.z -= 1;
          break;
        case core::math::Ray::Face::BACK:
          block.z += 1;
          break;
        case core::math::Ray::Face::LEFT:
          block.x -= 1;
          break;
        case core::math::Ray::Face::RIGHT:
          block.x += 1;
          break;
        case core::math::Ray::Face::TOP:
          block.y += 1;
          break;
        case core::math::Ray::Face::BOTTOM:
          block.y -= 1;
          break;
        }

        try {
          world.place_block(block, block::Type::GRASS);
        } catch (const core::VulkanKraftException &e) {
          core::Log::warning(std::string("failed to place block: ") + e.what());
        }
      }
    }
  }

  if (window.get_mouse().button_just_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    window.lock_cursor();
  }

  if (window.key_just_pressed(GLFW_KEY_ESCAPE)) {
    window.release_cursor();
  }

  // Update crosshair
  const auto [width, height] = window.get_framebuffer_size();
  m_crosshair.set_model_matrix(
      glm::vec2(static_cast<float>(width / 2), static_cast<float>(height / 2)),
      glm::vec2(crosshair_scale, crosshair_scale));
}

void Player::render(const core::vulkan::RenderCall &render_call) {
  m_crosshair.render(render_call);
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

glm::vec3 Player::_get_eye_position() const {
  return m_position + glm::vec3(0.0f, eye_height, 0.0f);
}