#include "../exception.hpp"
#include "../fps_timer.hpp"
#include "../log.hpp"
#include "../render_2d.hpp"
#include "../resource_hodler.hpp"
#include "../settings.hpp"
#include "../shader.hpp"
#include "../texture.hpp"
#include "../vulkan/buffer.hpp"
#include "../vulkan/context.hpp"
#include "../window.hpp"
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

    core::Render2D::set_shader(texture_2d_shader);
    core::Render2D block_texture_render(&block_texture);

    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();
      window.reset_keys();

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        {
          const auto [width, height] = window.get_framebuffer_size();
          block_texture_render.set_model_matrix(glm::vec2(
              static_cast<float>(width / 2), static_cast<float>(height / 2)));
          core::Render2D::update_projection_matrix(width, height, render_call);
        }
        core::Render2D::bind_shader(render_call);
        block_texture_render.render(render_call);
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}