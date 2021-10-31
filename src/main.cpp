#include "chunk/world.hpp"
#include "core/exception.hpp"
#include "core/fps_timer.hpp"
#include "core/log.hpp"
#include "core/settings.hpp"
#include "core/text/font.hpp"
#include "core/text/text.hpp"
#include "core/vulkan/context.hpp"
#include "core/window.hpp"
#include "main_menu_scene.hpp"
#include "player.hpp"
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <sstream>

static constexpr int fullscreen_keyboard_button = GLFW_KEY_F11;

int main(int args, char *argv[]) {
  // Initialise all objects not requireing vulkan directly
  core::Settings settings;
  core::FPSTimer timer(settings.max_fps);
  core::text::Text::GlobalUniform text_global;
  // The projection matrix for everything in the 3D space
  glm::mat4 projection_matrix;

  try {
    // Initialise the window
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    window.update_controller_db(settings.get_controller_db_file_name());
    // Initialise vulkan
    core::vulkan::Context context(window, settings);
    // Load all resources (textures, fonts, shaders, etc.)
    core::ResourceHodler hodler(context, settings);

    // Retrieve the shaders from the resource hodler
    auto &chunk_shader =
        hodler.get_shader(core::ResourceHodler::chunk_mesh_shader_name);
    auto &text_shader =
        hodler.get_shader(core::ResourceHodler::text_shader_name);
    auto &texture_2d_shader =
        hodler.get_shader(core::ResourceHodler::texture_2d_shader_name);

    core::Render2D::set_shader(texture_2d_shader);
    core::text::Text::set_shader(text_shader);

    std::unique_ptr<scene::Scene> current_scene(
        std::make_unique<MainMenuScene>(context, hodler, settings, window));

    // The game loop
    while (!window.should_close()) {
      // Begin measuring the frame time
      auto delta_timer(timer.begin_frame());
      window.poll_events();

      if (window.key_just_pressed(fullscreen_keyboard_button)) {
        window.toggle_fullscreen();
      }

      {
        auto switch_scene(
            current_scene->update(window, timer.get_delta_time()));
        if (switch_scene) {
          current_scene = std::move(switch_scene);
        }
      }

      window.reset_keys();

      // Start rendering the frame
      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        // Update the projection matrices
        const auto [width, height] = window.get_framebuffer_size();

        auto proj_2d(core::Render2D::update_projection_matrix(width, height,
                                                              render_call));
        text_global.proj = std::move(proj_2d);
        text_shader.update_uniform_buffer(render_call, text_global);

        current_scene->render(render_call, timer.get_delta_time());
      }
    }

  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}