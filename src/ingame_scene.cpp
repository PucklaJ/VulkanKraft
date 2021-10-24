#include "ingame_scene.hpp"
#include "glm/gtx/transform.hpp"

InGameScene::InGameScene(const core::vulkan::Context &context,
                         core::ResourceHodler &hodler,
                         const core::Settings &settings)
    : m_physics_server(1.0f / static_cast<float>(settings.max_fps)),
      m_fps_text(
          context, hodler.get_shader(core::ResourceHodler::text_shader_name),
          hodler.get_font(core::ResourceHodler::debug_font_name), L"60 FPS"),
      m_position_text(
          context, hodler.get_shader(core::ResourceHodler::text_shader_name),
          hodler.get_font(core::ResourceHodler::debug_font_name), L"X\nY\nZ\n"),
      m_look_text(context,
                  hodler.get_shader(core::ResourceHodler::text_shader_name),
                  hodler.get_font(core::ResourceHodler::debug_font_name),
                  L"Look\nX\nY\nZ"),
      m_vel_text(
          context, hodler.get_shader(core::ResourceHodler::text_shader_name),
          hodler.get_font(core::ResourceHodler::debug_font_name), L"Velocity"),
      m_player(glm::vec3(128.0f, 70.0f, 128.0f), hodler, m_physics_server),
      m_world(context, m_block_server),
      m_chunk_shader(
          hodler.get_shader(core::ResourceHodler::chunk_mesh_shader_name)),
      m_text_shader(hodler.get_shader(core::ResourceHodler::text_shader_name)),
      m_fov(settings.field_of_view) {

  m_world.set_center_position(m_player.position);
  m_fog_max_distance = m_world.set_render_distance(settings.render_distance);
  m_world.start_update_thread();
  // Wait until some chunks have been generated
  m_world.wait_for_generation(settings.render_distance *
                              settings.render_distance);
  // Set Player Data
  if (const auto player_data(m_world.get_save_world()->read_player_data());
      player_data) {
    m_player.position = player_data->position;
    m_player.velocity = player_data->velocity;
    m_player.set_rotation(player_data->rotation);
  } else if (const auto player_height(m_world.get_height(m_player.position));
             player_height) {
    // Place the player at the correct height
    m_player.position.y = static_cast<float>(*player_height);
  }
}

InGameScene::~InGameScene() {
  // Save player data
  const save::World::PlayerData player_data{
      m_player.position, m_player.velocity, m_player.get_rotation()};
  m_world.get_save_world()->write_player_data(player_data);
}

std::unique_ptr<scene::Scene> InGameScene::update(core::Window &window,
                                                  const float delta_time) {
  // Generate the texts for every text element
  {
    std::wstringstream fps_stream;
    fps_stream << std::setprecision(0) << std::fixed << (1.0f / delta_time);
    fps_stream << L" FPS";
    m_fps_text.set_string(fps_stream.str());
  }
  {
    constexpr auto float_width = 8;
    std::wstringstream pos_stream;
    pos_stream << std::fixed << std::setprecision(1);
    pos_stream << "X:" << std::setw(float_width) << std::right
               << m_player.position.x << std::endl;
    pos_stream << "Y:" << std::setw(float_width) << std::right
               << m_player.position.y << std::endl;
    pos_stream << "Z:" << std::setw(float_width) << std::right
               << m_player.position.z << std::endl;
    m_position_text.set_string(pos_stream.str());
  }
  {
    const auto look_dir(m_player.get_look_direction());
    constexpr auto float_width = 8;
    std::wstringstream stream;
    stream << "Look" << std::endl;
    stream << std::fixed << std::setprecision(3);
    stream << "X:" << std::setw(float_width) << std::right << look_dir.x
           << std::endl;
    stream << "Y:" << std::setw(float_width) << std::right << look_dir.y
           << std::endl;
    stream << "Z:" << std::setw(float_width) << std::right << look_dir.z
           << std::endl;
    m_look_text.set_string(stream.str());
  }
  {
    constexpr auto float_width = 8;
    std::wstringstream stream;
    stream << "Velocity" << std::endl;
    stream << std::fixed << std::setprecision(3);
    stream << "X:" << std::setw(float_width) << std::right
           << m_player.velocity.x << std::endl;
    stream << "Y:" << std::setw(float_width) << std::right
           << m_player.velocity.y << std::endl;
    stream << "Z:" << std::setw(float_width) << std::right
           << m_player.velocity.z << std::endl;
    m_vel_text.set_string(stream.str());
  }

  // Some debug input for testing purposes
  if (window.key_just_pressed(reseed_keyboard_button)) {
    m_world.clear_and_reseed();
  } else if (window.key_just_pressed(place_player_keyboard_button)) {
    // Place player at the correct height at the current position
    if (const auto player_height(m_world.get_height(m_player.position));
        player_height) {
      m_player.position.y = static_cast<float>(*player_height);
    }
  }

  m_player.update(window, m_world);
  m_world.set_center_position(m_player.position);

  m_physics_server.update(m_world, delta_time);

  // Update the projection matrices
  const auto [width, height] = window.get_framebuffer_size();
  m_chunk_global.proj_view = glm::perspective(
      m_fov, static_cast<float>(width) / static_cast<float>(height),
      core::Settings::near_plane, core::Settings::far_plane);
  m_chunk_global.proj_view[1][1] *= -1.0f;

  // Update the uniforms
  m_chunk_global.proj_view =
      m_chunk_global.proj_view * m_player.create_view_matrix();
  m_chunk_global.eye_pos = m_player.get_eye_position();

  return nullptr;
}

void InGameScene::render(const core::vulkan::RenderCall &render_call,
                         const float delta_time) {
  m_chunk_shader.update_uniform_buffer(render_call, m_chunk_global);

  // Render the world
  m_chunk_shader.bind(render_call);
  // fog max distance
  m_chunk_shader.set_push_constant(render_call, m_fog_max_distance);
  m_world.render(render_call);

  // Render the player
  core::Render2D::bind_shader(render_call);
  m_player.render(render_call);

  // Render the text elements
  m_text_shader.bind(render_call);
  m_fps_text.render(render_call);
  m_position_text.render(render_call);
  m_look_text.render(render_call);
  m_vel_text.render(render_call);
}