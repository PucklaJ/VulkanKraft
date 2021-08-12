#include "../../core/exception.hpp"
#include "../../core/fps_timer.hpp"
#include "../../core/log.hpp"
#include "../../core/settings.hpp"
#include "../../core/shader.hpp"
#include "../../core/texture.hpp"
#include "../../core/vulkan/buffer.hpp"
#include "../../core/vulkan/context.hpp"
#include "../../core/window.hpp"
#include "../perlin_noise.hpp"
#include <glm/gtx/transform.hpp>

#include <shaders/perlin_noise_test_frag.hpp>
#include <shaders/perlin_noise_test_vert.hpp>

int main(int args, char *argv[]) {
  core::Settings settings;
  core::FPSTimer timer(settings.max_fps);
  world_gen::PerlinNoise noise;

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);

    constexpr auto res = 1024;

    std::array<uint8_t, res * res * 4> pixel_data;
    for (int x = 0; x < res; x++) {
      for (int y = 0; y < res; y++) {
        auto noise_value{noise.get(x - res / 2, y, 0.02f, 8)};

        noise_value += 1.0f;
        noise_value /= 2.0f;

        // Red
        pixel_data[y * res * 4 + x * 4 + 0] = noise_value * 255;
        // Green
        pixel_data[y * res * 4 + x * 4 + 1] = noise_value * 255;
        // Blue
        pixel_data[y * res * 4 + x * 4 + 2] = noise_value * 255;
        // Alpha
        pixel_data[y * res * 4 + x * 4 + 3] = 255;
      }
    }

    const auto noise_texture(core::Texture::Builder()
                                 .dimensions(res, res)
                                 .filter(vk::Filter::eNearest)
                                 .build(context, pixel_data.data()));

    auto noise_shader(core::Shader::Builder()
                          .vertex(perlin_noise_test_vert_spv)
                          .fragment(perlin_noise_test_frag_spv)
                          .texture()
                          .build(context, settings));
    noise_shader.set_texture(noise_texture);

    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();
      window.reset_keys();

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};

        noise_shader.bind(render_call);
        render_call.render_vertices(6);
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}