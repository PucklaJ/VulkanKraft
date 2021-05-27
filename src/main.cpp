#include "core/exception.hpp"
#include "core/log.hpp"
#include "core/shader.hpp"
#include "core/vulkan/buffer.hpp"
#include "core/vulkan/context.hpp"
#include "core/vulkan/vertex.hpp"
#include "core/window.hpp"
#include <chrono>
#include <fstream>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

constexpr uint32_t window_width = 840;
constexpr uint32_t window_height = 480;
constexpr char window_title[] = "VulkanKraft";

int main(int args, char *argv[]) {
  const auto vertices = std::array{core::vulkan::Vertex(-0.5f, 0.5f, -1.0f),
                                   core::vulkan::Vertex(0.5f, 0.5f, -1.0f),
                                   core::vulkan::Vertex(0.5f, -0.5f, -1.0f),
                                   core::vulkan::Vertex(0.5f, -0.5f, -1.0f),
                                   core::vulkan::Vertex(-0.5f, -0.5f, -1.0f),
                                   core::vulkan::Vertex(-0.5f, 0.5f, -1.0f)};

  try {
    core::Window window(window_width, window_height, window_title);
    core::vulkan::Context context(window);

    auto shader = core::Shader::Builder()
                      .vertex("shaders_spv/triangle.vert.spv")
                      .fragment("shaders_spv/triangle.frag.spv")
                      .add_uniform_buffer(vk::ShaderStageFlagBits::eVertex)
                      .build(context);

    core::vulkan::Buffer vertex_buffer(context,
                                       vk::BufferUsageFlagBits::eVertexBuffer,
                                       sizeof(vertices), vertices.data());

    core::Shader::MatrixData ubo0;
    ubo0.view = glm::identity<glm::mat4>();

    while (!window.should_close()) {
      window.poll_events();

      const auto [width, height] = window.get_framebuffer_size();
      ubo0.proj = glm::perspective(
          glm::degrees(15.0f),
          static_cast<float>(width) / static_cast<float>(height), 0.01f, 2.0f);
      ubo0.model =
          glm::rotate(glm::radians(90.0f * static_cast<float>(glfwGetTime())),
                      glm::vec3(0.0f, 0.0f, 1.0f));

      if (const auto render_call = context.render_begin(); render_call) {
        shader.update_uniform_buffer(render_call.value(), ubo0);
        shader.bind(render_call.value());
        vertex_buffer.bind(render_call.value());
        render_call.value().render_vertices(vertices.size());
      }

      context.render_end();

      std::this_thread::sleep_for(10ms);
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
    return 1;
  }

  return 0;
}