#include "../../core/exception.hpp"
#include "../../core/fps_timer.hpp"
#include "../../core/log.hpp"
#include "../../core/resource_hodler.hpp"
#include "../../core/settings.hpp"
#include "../../core/shader.hpp"
#include "../../core/texture.hpp"
#include "../../core/vulkan/buffer.hpp"
#include "../../core/vulkan/context.hpp"
#include "../../core/window.hpp"
#include <glm/gtx/transform.hpp>

int main(int args, char *argv[]) {
  core::Settings settings;
  core::FPSTimer timer(settings.max_fps);

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);
    core::ResourceHodler hodler(context, settings);

    auto &block_texture =
        hodler.get_texture(core::ResourceHodler::chunk_mesh_texture_name);
    auto &texture_2d_shader =
        hodler.get_shader(core::ResourceHodler::texture_2d_shader_name);

    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();
      window.reset_keys();

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        {
          const auto [width, height] = window.get_framebuffer_size();
          const auto proj(glm::ortho(0.0f, static_cast<float>(width), 0.0f,
                                     static_cast<float>(height)));
          // Translate it to the middle of the screen and apply the size of the
          // texture
          const auto model(
              glm::translate(glm::vec3(static_cast<float>(width / 2),
                                       static_cast<float>(height / 2), 0.0f)) *
              glm::scale(glm::vec3(
                  static_cast<float>(block_texture.get_width() / 2),
                  static_cast<float>(block_texture.get_height() / 2), 1.0f)));
          texture_2d_shader.update_uniform_buffer(render_call, proj * model);
        }
        texture_2d_shader.bind(render_call);
        texture_2d_shader.bind_dynamic_texture(render_call, block_texture);
        render_call.render_vertices(6);
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}