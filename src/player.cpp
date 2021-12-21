#include "player.hpp"
#include "core/exception.hpp"
#include "core/log.hpp"
#include "physics/ray.hpp"
#include <glm/gtx/transform.hpp>

Player::Player(const glm::vec3 &position, core::ResourceHodler &hodler,
               physics::Server &physics_server)
    : physics::MovingObject(
          position, glm::vec3(aabb_width, eye_height, aabb_depth),
          glm::vec3(-aabb_width / 2.0f, 0.0f, -aabb_depth / 2.0f)),
      m_rotation(0.0f, 0.0f), m_last_mouse_x(0.0f), m_last_mouse_y(0.0f),
      m_last_left_trigger(false), m_last_right_trigger(false),
      m_crosshair(
          &hodler.get_texture(core::ResourceHodler::crosshair_texture_name)) {
  physics_server.add_mob(this);
}

glm::mat4 Player::create_view_matrix() const {
  const auto eye_position(get_eye_position());
  const auto look_direction(get_look_direction());

  return glm::lookAt(eye_position, eye_position + look_direction,
                     glm::vec3(0.0f, 1.0f, 0.0f));
}

void Player::update(core::Window &window, chunk::World &world) {
  // ***** handle input ************
  bool button_jump, button_down, button_place, button_destroy;
  glm::vec2 move_direction(0.0f, 0.0f), view(0.0f, 0.0f);

  _update_input(window, button_jump, button_down, button_place, button_destroy,
                move_direction, view);
  // *******************************

  // ***** handle camera ***********
  m_rotation.x += -view.x * glm::radians(0.1f);
  m_rotation.y += -view.y * glm::radians(0.1f);
  m_rotation.y = glm::clamp(m_rotation.y, min_y_rot, max_y_rot);
  // *******************************

  // **** handle movement *****
  const auto look_direction(get_look_direction());
  const auto forward(
      glm::normalize(glm::vec3(look_direction.x, 0.0f, look_direction.z)));
  const auto right_direction(
      glm::normalize(glm::cross(look_direction, glm::vec3(0.0f, 1.0f, 0.0f))));

  velocity.x = 0.0f;
  velocity.z = 0.0f;
  velocity += forward * move_direction.y * move_speed;
  velocity += right_direction * move_direction.x * move_speed;
  // **** handle jump ****
  {
    const auto should_jump{button_jump && _is_grounded(world)};
    velocity.y = should_jump * jump_power + !should_jump * velocity.y;
  }
  // *********************
  // **************************

  // ******** handle block place/destroy *******
  if (button_destroy) {
    // Create a ray at the eyes and cast it along the look direction
    const physics::Ray ray{get_eye_position(), look_direction};
    physics::Ray::Face face;
    float _;
    const auto _block(world.raycast_block(ray, face, _));
    if (_block) {
      const auto block(*_block);

      try {
        world.destroy_block(block);
      } catch (const core::VulkanKraftException &e) {
        core::Log::warning(std::string("failed to destroy block: ") + e.what());
      }
    }
  } else if (button_place) {
    // Create a ray at the eyes and cast it along the look direction
    const physics::Ray ray{get_eye_position(), look_direction};
    physics::Ray::Face face;
    float _;
    const auto _block(world.raycast_block(ray, face, _));
    if (_block) {
      auto block(*_block);

      // Determine the position of the block the will be newly created based on
      // the face the player is facing
      switch (face) {
      case physics::Ray::Face::FRONT:
        block.z -= 1;
        break;
      case physics::Ray::Face::BACK:
        block.z += 1;
        break;
      case physics::Ray::Face::LEFT:
        block.x -= 1;
        break;
      case physics::Ray::Face::RIGHT:
        block.x += 1;
        break;
      case physics::Ray::Face::TOP:
        block.y += 1;
        break;
      case physics::Ray::Face::BOTTOM:
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
  // *******************************************

  // ***** handle cursor lock and release ******
  if (window.get_mouse().button_just_pressed(lock_cursor_mouse_button)) {
    window.lock_cursor();
  }

  if (window.key_just_pressed(unlock_cursor_keyboard_button)) {
    window.release_cursor();
  }
  // *******************************************

  // Update crosshair
  const auto [width, height] = window.get_framebuffer_size();
  m_crosshair.set_model_matrix(
      glm::vec2(static_cast<float>(width / 2), static_cast<float>(height / 2)),
      glm::vec2(crosshair_scale, crosshair_scale));
}

void Player::render(const core::vulkan::RenderCall &render_call) {
  m_crosshair.render(render_call);
}

std::optional<glm::ivec3>
Player::get_selected_block_position(chunk::World &world) const {
  const physics::Ray ray{get_eye_position(), get_look_direction()};
  physics::Ray::Face face;
  float _;
  return world.raycast_block(ray, face, _);
}

void Player::_update_input(core::Window &window, bool &button_jump,
                           bool &button_down, bool &button_place,
                           bool &button_destroy, glm::vec2 &move_direction,
                           glm::vec2 &view) {

  // Gamepad input
  if (window.is_gamepad_connected()) {
    constexpr float axis_dead_zone = 0.1f;
    constexpr float trigger_threshold = 0.3f;

    // **** handle left stick ******
    if (const auto horizontal_value(
            window.get_gamepad_axis_value(move_horizontal_gamepad_axis));
        horizontal_value) {
      move_direction.x = _abs_dead_zone(*horizontal_value, axis_dead_zone);
    }
    if (const auto vertical_value(
            window.get_gamepad_axis_value(move_vertical_gamepad_axis));
        vertical_value) {
      move_direction.y = _abs_dead_zone(-*vertical_value, axis_dead_zone);
    }
    // ******************************

    // ***** handle trigger input ***
    if (const auto right_trigger(
            window.get_gamepad_axis_value(block_place_gamepad_axis));
        right_trigger && *right_trigger > trigger_threshold) {
      button_place = !m_last_right_trigger;
      m_last_right_trigger = true;
    } else {
      m_last_right_trigger = false;
      button_place = false;
    }

    if (const auto left_trigger(
            window.get_gamepad_axis_value(block_destroy_gamepad_axis));
        left_trigger && *left_trigger > trigger_threshold) {
      button_destroy = !m_last_left_trigger;
      m_last_left_trigger = true;
    } else {
      m_last_left_trigger = false;
      button_destroy = false;
    }
    // ******************************

    button_jump = window.gamepad_button_just_pressed(jump_gamepad_button);
    button_down = window.gamepad_button_is_pressed(sneak_gamepad_button);

    constexpr float axis_view_factor = 20.0f;

    // ***** handle right stick ******
    if (const auto horizontal_value{
            window.get_gamepad_axis_value(look_horizontal_gamepad_axis)};
        horizontal_value) {
      view.x =
          _abs_dead_zone(*horizontal_value, axis_dead_zone) * axis_view_factor;
    }
    if (const auto vertical_value{
            window.get_gamepad_axis_value(look_vertical_gamepad_axis)};
        vertical_value) {
      view.y =
          _abs_dead_zone(*vertical_value, axis_dead_zone) * axis_view_factor;
    }
    // ********************************
  } else {
    button_jump = false;
    button_place = false;
    button_down = false;
    button_destroy = false;
  }

  // Mouse and Keyboard input
  if (const auto y_move{window.key_is_pressed(move_forward_keyboard_button) -
                        window.key_is_pressed(move_backward_keyboard_button)};
      y_move != 0) {
    move_direction.y = y_move;
  }
  if (const auto x_move{window.key_is_pressed(move_right_keyboard_button) -
                        window.key_is_pressed(move_left_keyboard_button)};
      x_move != 0) {
    move_direction.x = x_move;
  }
  button_jump = button_jump || window.key_just_pressed(jump_keyboard_button);
  button_down = button_down || window.key_is_pressed(sneak_keyboard_button);
  button_place =
      button_place ||
      (window.cursor_is_locked() &&
       window.get_mouse().button_just_pressed(block_place_mouse_button));
  button_destroy =
      button_destroy ||
      (window.cursor_is_locked() &&
       window.get_mouse().button_just_pressed(block_destroy_mouse_button));

  // **** handle mouse input *****
  const auto cur_mouse_x = window.get_mouse().screen_position.x;
  const auto cur_mouse_y = window.get_mouse().screen_position.y;
  if (window.cursor_is_locked()) {
    if (const auto x_view{cur_mouse_x - m_last_mouse_x}; x_view != 0.0f) {
      view.x = x_view;
    }
    if (const auto y_view{cur_mouse_y - m_last_mouse_y}; y_view != 0.0f) {
      view.y = y_view;
    }
  }
  m_last_mouse_x = cur_mouse_x;
  m_last_mouse_y = cur_mouse_y;
  // *****************************
}

bool Player::_is_grounded(chunk::World &world) const {
  // Use four rays on each corner of the AABB
  const auto ground_rays = std::array{
      physics::Ray{position + glm::vec3(-aabb_width, 0.0f, -aabb_depth),
                   glm::vec3(0.0f, -1.0f, 0.0f)},
      physics::Ray{position + glm::vec3(+aabb_width, 0.0f, -aabb_depth),
                   glm::vec3(0.0f, -1.0f, 0.0f)},
      physics::Ray{position + glm::vec3(-aabb_width, 0.0f, +aabb_depth),
                   glm::vec3(0.0f, -1.0f, 0.0f)},
      physics::Ray{position + glm::vec3(+aabb_width, 0.0f, +aabb_depth),
                   glm::vec3(0.0f, -1.0f, 0.0f)}};

  physics::Ray::Face _;
  float distance;

  for (const auto &ground_ray : ground_rays) {
    const auto hit_block(world.raycast_block(ground_ray, _, distance));
    if (hit_block && distance < max_ground_ray_distance) {
      return true;
    }
  }

  return false;
}

glm::vec3 Player::get_look_direction() const {
  // Default look direction when no rotation is applied is the negative z axis
  glm::vec3 look_direction(0.0f, 0.0f, -1.0f);

  // Apply the rotation around the y axis (based on horizontal mouse movement)
  const auto y_rot_mat(glm::rotate(m_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f)));
  look_direction = y_rot_mat * glm::vec4(look_direction, 1.0f);

  // Change the x axis based on the rotation
  const glm::vec3 x_axis(y_rot_mat * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
  // Apply the rotation around the x axis (based on vertical mouse movement)
  look_direction =
      glm::rotate(m_rotation.y, x_axis) * glm::vec4(look_direction, 1.0f);
  return look_direction;
}
