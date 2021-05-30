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
  m_vertex_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eVertexBuffer, 1);
  m_index_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eIndexBuffer, 1);
}

void Mesh::render(const ::core::vulkan::RenderCall &render_call) {
  m_vertex_buffer->bind(render_call);
  m_index_buffer->bind(render_call);
  render_call.render_indices(m_num_indices);
}

void Mesh::_create_cube(std::vector<Mesh::Vertex> &vertices,
                        std::vector<uint32_t> &indices, const glm::vec3 &p,
                        const bool front_face, const bool back_face,
                        const bool left_face, const bool right_face,
                        const bool top_face, const bool bot_face) {
  vertices.reserve(vertices.size() + 8);

  const auto i{vertices.size()};
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f);
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f);
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f);
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f);
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f);
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f);
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f);
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f);

  if (front_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 0);
  }

  if (back_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 6);
    indices.emplace_back(i + 5);
    indices.emplace_back(i + 4);
    indices.emplace_back(i + 4);
    indices.emplace_back(i + 7);
    indices.emplace_back(i + 6);
  }

  if (right_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 5);
    indices.emplace_back(i + 6);
    indices.emplace_back(i + 6);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
  }

  if (left_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 4);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 7);
    indices.emplace_back(i + 4);
  }

  if (top_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 6);
    indices.emplace_back(i + 6);
    indices.emplace_back(i + 7);
    indices.emplace_back(i + 3);
  }

  if (bot_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 4);
    indices.emplace_back(i + 4);
    indices.emplace_back(i + 5);
    indices.emplace_back(i + 1);
  }
}
}; // namespace chunk