#include "chunk/mesh.hpp"
#include "core/exception.hpp"
#include "core/fps_timer.hpp"
#include "core/log.hpp"
#include "core/settings.hpp"
#include "core/vulkan/context.hpp"
#include "core/window.hpp"

int main(int args, char *argv[]) {
  core::Settings settings;
  core::FPSTimer timer(settings);

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);
    auto chunk_mesh_shader = chunk::Mesh::build_shader(context, settings);
    chunk::Mesh chunk_mesh(context);

    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};
        chunk_mesh_shader.bind(render_call);
        chunk_mesh.render(render_call);
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}