#include "chunk/mesh.hpp"
#include "core/exception.hpp"
#include "core/fps_timer.hpp"
#include "core/log.hpp"
#include "core/settings.hpp"
#include "core/vulkan/context.hpp"
#include "core/window.hpp"
#include <glm/gtx/transform.hpp>

int main(int args, char *argv[]) {
  core::Settings settings;
  core::FPSTimer timer(settings);
  chunk::Mesh::GlobalUniform chunk_mesh_global;
  glm::mat4 projection_matrix;
  glm::mat4 view_matrix =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);
    auto chunk_mesh_shader = chunk::Mesh::build_shader(context, settings);
    chunk::Mesh chunk_mesh(context);

    float current_time = 0.0f;
    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();

      const auto [width, height] = window.get_framebuffer_size();
      projection_matrix = glm::perspective(
          settings.field_of_view,
          static_cast<float>(width) / static_cast<float>(height),
          core::Settings::near_plane, core::Settings::far_plane);
      projection_matrix[1][1] *= -1.0f;
      chunk_mesh_global.proj_view = projection_matrix * view_matrix;

      if (const auto _render_call(context.render_begin()); _render_call) {
        const auto &render_call{*_render_call};
        chunk_mesh_shader.update_uniform_buffer(render_call, chunk_mesh_global);
        chunk_mesh_shader.bind(render_call);
        chunk_mesh.render(render_call);
      }
      current_time += timer.get_delta_time();
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}