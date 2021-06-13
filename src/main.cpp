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
  core::Settings settings;
  core::FPSTimer timer(settings.max_fps);
  chunk::GlobalUniform chunk_global;
  core::text::Text::GlobalUniform text_global;
  glm::mat4 projection_matrix;
  glm::mat4 text_projection_matrix;

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);
    core::ResourceHodler hodler(context);

    auto chunk_shader = chunk::Mesh::build_shader(context, settings, hodler);
    auto text_shader = core::text::Text::build_shader(context, settings);

    core::text::Font font("fonts/Mister Pixel Regular.otf");
    core::text::Text fps_text(context, text_shader, font, L"60 FPS");
    core::text::Text position_text(context, text_shader, font, L"X\nY\nZ\n",
                                   glm::vec2(0.0f, fps_text.get_height() + 10));
    core::text::Text look_text(context, text_shader, font, L"Look",
                               glm::vec2(0.0f, fps_text.get_height() + 10 +
                                                   position_text.get_height() +
                                                   10));
    core::text::Text cross_hair(
        context, text_shader, font, L".",
        glm::vec2(settings.window_width / 2, settings.window_height / 2));

    Player player(glm::vec3(8.0f, 70.0f, 8.0f));
    chunk::World world(context);

    world.set_center_position(player.get_position());
    world.set_render_distance(settings.render_distance);
    world.start_update_thread();

    float current_time = 0.0f;
    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();

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
                   << player.get_position().x << std::endl;
        pos_stream << "Y:" << std::setw(float_width) << std::right
                   << player.get_position().y << std::endl;
        pos_stream << "Z:" << std::setw(float_width) << std::right
                   << player.get_position().z << std::endl;
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

      player.update(timer, window, world);
      world.set_center_position(player.get_position());

      window.reset_keys();

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        const auto [width, height] = window.get_framebuffer_size();
        projection_matrix = glm::perspective(
            settings.field_of_view,
            static_cast<float>(width) / static_cast<float>(height),
            core::Settings::near_plane, core::Settings::far_plane);
        projection_matrix[1][1] *= -1.0f;

        text_projection_matrix = glm::ortho(0.0f, static_cast<float>(width),
                                            0.0f, static_cast<float>(height));

        chunk_global.proj_view =
            projection_matrix * player.create_view_matrix();
        chunk_shader.update_uniform_buffer(render_call, chunk_global);
        text_global.proj = text_projection_matrix;
        text_shader.update_uniform_buffer(render_call, text_global);

        chunk_shader.bind(render_call);
        world.render(render_call);

        text_shader.bind(render_call);
        fps_text.render(render_call);
        position_text.render(render_call);
        look_text.render(render_call);
        cross_hair.render(render_call);
      }
      current_time += timer.get_delta_time();
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}