#include "mesh.hpp"
#include <array>

namespace chunk {
Mesh::Vertex::Vertex(float x, float y, float z) : position(x, y, z) {}

::core::Shader Mesh::build_shader(const ::core::vulkan::Context &context,
                                  const ::core::Settings &settings) {
  return ::core::Shader::Builder()
      .vertex_attribute<glm::vec3>()
      .vertex("shaders_spv/chunk_mesh.vert.spv")
      .fragment("shaders_spv/chunk_mesh.frag.spv")
      .build(context, settings);
}

Mesh::Mesh(const ::core::vulkan::Context &context) {
  const auto vertices =
      std::array{Vertex(-0.5f, 0.5f, 0.0f), Vertex(0.5f, 0.5f, 0.0f),
                 Vertex(0.5f, -0.5f, 0.0f), Vertex(-0.5f, -0.5f, 0.0f)};
  const auto indices =
      std::array{static_cast<uint32_t>(0), static_cast<uint32_t>(1),
                 static_cast<uint32_t>(2), static_cast<uint32_t>(2),
                 static_cast<uint32_t>(3), static_cast<uint32_t>(0)};
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