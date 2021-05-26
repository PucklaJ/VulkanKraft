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
  const auto vertices = std::array{core::vulkan::Vertex(-0.5f, 0.5f, 0.0f),
                                   core::vulkan::Vertex(0.5f, 0.5f, 0.0f),
                                   core::vulkan::Vertex(0.5f, -0.5f, 0.0f),
                                   core::vulkan::Vertex(0.5f, -0.5f, 0.0f),
                                   core::vulkan::Vertex(-0.5f, -0.5f, 0.0f),
                                   core::vulkan::Vertex(-0.5f, 0.5f, 0.0f)};

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

    while (!window.should_close()) {
      window.poll_events();

      context.render_begin();

      shader.bind();
      vertex_buffer.bind();
      context.render_vertices(vertices.size());

      context.render_end();

      std::this_thread::sleep_for(10ms);
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
    return 1;
  }

  return 0;
}