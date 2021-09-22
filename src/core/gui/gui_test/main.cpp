#include "../../exception.hpp"
#include "../../fps_timer.hpp"
#include "../../log.hpp"
#include "../../resource_hodler.hpp"
#include "../../settings.hpp"
#include "../../shader.hpp"
#include "../../texture.hpp"
#include "../../vulkan/context.hpp"
#include "../../window.hpp"
#include "../button.hpp"
#include "../context.hpp"
#include <glm/gtx/transform.hpp>

int main(int args, char *argv[]) {
  core::Settings settings;
  core::FPSTimer timer(settings.max_fps);

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);
    core::ResourceHodler hodler(context, settings);

    auto &texture_2d_shader =
        hodler.get_shader(core::ResourceHodler::texture_2d_shader_name);
    auto &text_shader =
        hodler.get_shader(core::ResourceHodler::text_shader_name);
    auto &text_font = hodler.get_font(core::ResourceHodler::debug_font_name);

    core::Render2D::set_shader(texture_2d_shader);
    core::gui::Context gui_context(context, window, text_shader, text_font);

    gui_context
        .add_element<core::gui::Button>(
            &gui_context, std::ref(hodler), L"Play", 250, 100,
            glm::vec2(settings.window_width / 2, settings.window_height / 2))
        ->on_click = std::bind(core::Log::info, "Play clicked");

    core::text::Text play_text(context, text_shader, text_font, L"Play");

    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();
      gui_context.update();

      window.reset_keys();

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        {
          const auto [width, height] = window.get_framebuffer_size();
          const auto proj(core::Render2D::update_projection_matrix(
              width, height, render_call));

          text_shader.update_uniform_buffer(render_call, proj);
        }

        gui_context.render(render_call);
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}