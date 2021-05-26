#include "core/exception.hpp"
#include "core/log.hpp"
#include "core/vulkan/buffer.hpp"
#include "core/vulkan/context.hpp"
#include "core/vulkan/graphics_pipeline.hpp"
#include "core/vulkan/vertex.hpp"
#include "core/window.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

constexpr uint32_t window_width = 840;
constexpr uint32_t window_height = 480;
constexpr char window_title[] = "VulkanKraft";

std::vector<char> read_file(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (file.fail()) {
    throw std::runtime_error("failed to open " + filename);
  }

  const auto fileSize = file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  return buffer;
}

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

    auto vertex_code(read_file("shaders_spv/triangle.vert.spv"));
    auto fragment_code(read_file("shaders_spv/triangle.frag.spv"));

    core::vulkan::GraphicsPipeline gp(context, std::move(vertex_code),
                                      std::move(fragment_code));

    core::vulkan::Buffer vertex_buffer(context,
                                       vk::BufferUsageFlagBits::eVertexBuffer,
                                       sizeof(vertices), vertices.data());

    while (!window.should_close()) {
      window.poll_events();

      context.render_begin();
      gp.bind();
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