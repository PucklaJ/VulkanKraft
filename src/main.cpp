#include "chunk/world.hpp"
#include "core/exception.hpp"
#include "core/fps_timer.hpp"
#include "core/log.hpp"
#include "core/settings.hpp"
#include "core/text/font.hpp"
#include "core/text/text.hpp"
#include "core/vulkan/context.hpp"
#include "core/window.hpp"
#include "player.hpp"
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <sstream>

int main(int args, char *argv[]) {
  // Initialise all objects not requireing vulkan directly
  core::Settings settings;
  core::FPSTimer timer(settings.max_fps);
  chunk::GlobalUniform chunk_global;
  core::text::Text::GlobalUniform text_global;
  // The projection matrix for everything in the 3D space
  glm::mat4 projection_matrix;

  try {
    // Initialise the window
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    // Initialise vulkan
    core::vulkan::Context context(window, settings);
    // Load all resources (textures, fonts, shaders, etc.)
    core::ResourceHodler hodler(context, settings);
    block::Server block_server;
    physics::Server physics_server(1.0f / static_cast<float>(settings.max_fps));

    // Retrieve the shaders from the resource hodler
    auto &chunk_shader =
        hodler.get_shader(core::ResourceHodler::chunk_mesh_shader_name);
    auto &text_shader =
        hodler.get_shader(core::ResourceHodler::text_shader_name);
    auto &texture_2d_shader =
        hodler.get_shader(core::ResourceHodler::texture_2d_shader_name);

    core::Render2D::set_shader(texture_2d_shader);

    // Retrieve the font from the resource hodler
    auto &debug_font = hodler.get_font(core::ResourceHodler::debug_font_name);
    // Text used to display the frames per second
    core::text::Text fps_text(context, text_shader, debug_font, L"60 FPS");
    // Text used to display the position of the player
    core::text::Text position_text(context, text_shader, debug_font,
                                   L"X\nY\nZ\n",
                                   glm::vec2(0.0f, fps_text.get_height() + 10));
    // Text used to display the look direction of the player
    core::text::Text look_text(context, text_shader, debug_font, L"Look",
                               glm::vec2(0.0f, fps_text.get_height() + 10 +
                                                   position_text.get_height() +
                                                   10));

    // Initialise the player and world
    Player player(glm::vec3(128.5f, 70.0f, 128.5f), hodler, physics_server);

    // Initialise all values of the world and start the background thread for
    // updating the chunks
    chunk::World world(context, block_server);
    world.set_center_position(player.position);
    const auto fog_max_distance{
        world.set_render_distance(settings.render_distance)};
    world.start_update_thread();
    // Wait until some chunks have been generated
    world.wait_for_generation(settings.render_distance *
                              settings.render_distance);

    // Place the player at the correct height
    if (const auto player_height(world.get_height(player.position));
        player_height) {
      player.position.y = static_cast<float>(*player_height);
    }

    // The game loop
    while (!window.should_close()) {
      // Begin measuring the frame time
      auto delta_timer(timer.begin_frame());
      window.poll_events();

      // Generate the texts for every text element
      {
        std::wstringstream fps_stream;
        fps_stream << std::setprecision(0) << std::fixed
                   << (1.0f / timer.get_delta_time());
        fps_stream << L" FPS";
        fps_text.set_string(fps_stream.str());
      }
      {
        constexpr auto float_width = 8;
        std::wstringstream pos_stream;
        pos_stream << std::fixed << std::setprecision(1);
        pos_stream << "X:" << std::setw(float_width) << std::right
                   << player.position.x << std::endl;
        pos_stream << "Y:" << std::setw(float_width) << std::right
                   << player.position.y << std::endl;
        pos_stream << "Z:" << std::setw(float_width) << std::right
                   << player.position.z << std::endl;
        position_text.set_string(pos_stream.str());
      }
      {
        const auto look_dir(player.get_look_direction());
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
        look_text.set_string(stream.str());
      }

      // Some debug input for testing purposes
      if (window.key_just_pressed(GLFW_KEY_F8)) {
        world.clear_and_reseed();
      } else if (window.key_just_pressed(GLFW_KEY_F9)) {
        // Place player at the correct height at the current position
        if (const auto player_height(world.get_height(player.position));
            player_height) {
          player.position.y = static_cast<float>(*player_height);
        }
      }

      if (window.key_just_pressed(GLFW_KEY_F11)) {
        window.toggle_fullscreen();
      }

      player.update(window, world);
      world.set_center_position(player.position);

      physics_server.update(world, timer.get_delta_time());
      window.reset_keys();

      // Start rendering the frame
      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        // Update the projection matrices
        const auto [width, height] = window.get_framebuffer_size();
        projection_matrix = glm::perspective(
            settings.field_of_view,
            static_cast<float>(width) / static_cast<float>(height),
            core::Settings::near_plane, core::Settings::far_plane);
        projection_matrix[1][1] *= -1.0f;

        // Update the uniforms
        chunk_global.proj_view =
            projection_matrix * player.create_view_matrix();
        chunk_global.eye_pos = player.get_eye_position();
        chunk_shader.update_uniform_buffer(render_call, chunk_global);
        auto proj_2d(core::Render2D::update_projection_matrix(width, height,
                                                              render_call));
        text_global.proj = std::move(proj_2d);
        text_shader.update_uniform_buffer(render_call, text_global);

        // Render the world
        chunk_shader.bind(render_call);
        // fog max distance
        chunk_shader.set_push_constant(render_call, fog_max_distance);
        world.render(render_call);

        // Render the player
        core::Render2D::bind_shader(render_call);
        player.render(render_call);

        // Render the text elements
        text_shader.bind(render_call);
        fps_text.render(render_call);
        position_text.render(render_call);
        look_text.render(render_call);
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}