#include "mesh.hpp"
#include <array>

namespace chunk {
Mesh::Vertex::Vertex(float x, float y, float z) : position(x, y, z) {}

::core::Shader Mesh::build_shader(const ::core::vulkan::Context &context,
                                  const ::core::Settings &settings) {
  GlobalUniform global;
  global.proj_view = glm::mat4(1.0f);

  return ::core::Shader::Builder()
      .vertex_attribute<glm::vec3>()
      .vertex("shaders_spv/chunk_mesh.vert.spv")
      .fragment("shaders_spv/chunk_mesh.frag.spv")
      .uniform_buffer(vk::ShaderStageFlagBits::eVertex, global)
      .build(context, settings);
}

Mesh::Mesh(const ::core::vulkan::Context &context) {
  const auto vertices =
      std::array{Vertex(-0.5f, 0.5f, -0.5f), Vertex(0.5f, 0.5f, -0.5f),
                 Vertex(0.5f, -0.5f, -0.5f), Vertex(-0.5f, -0.5f, -0.5f),
                 Vertex(-0.5f, 0.5f, 0.5f),  Vertex(0.5f, 0.5f, 0.5f),
                 Vertex(0.5f, -0.5f, 0.5f),  Vertex(-0.5f, -0.5f, 0.5f)};
  const auto indices =
      std::array{static_cast<uint32_t>(0), static_cast<uint32_t>(1),
                 static_cast<uint32_t>(2), static_cast<uint32_t>(2),
                 static_cast<uint32_t>(3), static_cast<uint32_t>(0),

                 static_cast<uint32_t>(6), static_cast<uint32_t>(5),
                 static_cast<uint32_t>(4), static_cast<uint32_t>(4),
                 static_cast<uint32_t>(7), static_cast<uint32_t>(6),

                 static_cast<uint32_t>(1), static_cast<uint32_t>(5),
                 static_cast<uint32_t>(6), static_cast<uint32_t>(6),
                 static_cast<uint32_t>(2), static_cast<uint32_t>(1),

                 static_cast<uint32_t>(4), static_cast<uint32_t>(0),
                 static_cast<uint32_t>(3), static_cast<uint32_t>(3),
                 static_cast<uint32_t>(7), static_cast<uint32_t>(4),

                 static_cast<uint32_t>(3), static_cast<uint32_t>(2),
                 static_cast<uint32_t>(6), static_cast<uint32_t>(6),
                 static_cast<uint32_t>(7), static_cast<uint32_t>(3),

                 static_cast<uint32_t>(1), static_cast<uint32_t>(0),
                 static_cast<uint32_t>(4), static_cast<uint32_t>(4),
                 static_cast<uint32_t>(5), static_cast<uint32_t>(1)};
  num_indices = static_cast<uint32_t>(indices.size());

  m_vertex_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(vertices),
      vertices.data());
  m_index_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eIndexBuffer, sizeof(indices),
      indices.data());
}

void Mesh::render(const ::core::vulkan::RenderCall &render_call) {
  m_vertex_buffer->bind(render_call);
  m_index_buffer->bind(render_call);
  render_call.render_indices(num_indices);
}
}; // namespace chunk