#include "mesh.hpp"
#include "../core/log.hpp"
#include "chunk.hpp"
#include <array>
#include <cstdlib>
#ifndef NDEBUG
#include <chrono>
#include <sstream>
#endif

namespace chunk {

Mesh::Mesh(const ::core::vulkan::Context &context)
    : m_num_indices(-1), m_context(context) {}

void Mesh::render(const ::core::vulkan::RenderCall &render_call) {
  if (!m_vertex_buffer)
    return;

  m_vertex_buffer->bind(render_call);
  m_index_buffer->bind(render_call);
  render_call.render_indices(m_num_indices);
}

void Mesh::generate_vertices(const block::Server &block_server, Chunk *chunk,
                             const glm::vec2 &pos) {
#ifndef NDEBUG
  const auto start_time(std::chrono::high_resolution_clock::now());
#endif
  if (m_num_indices != -1) {
    m_vertices.reserve(m_num_indices / Block::indices_per_face *
                       Block::vertices_per_face);
    m_indices.reserve(m_num_indices);
  } else {
    m_vertices.reserve(Block::default_face_count * Block::vertices_per_face);
    m_indices.reserve(Block::default_face_count * Block::indices_per_face);
  }

  for (size_t x = 0; x < block_width; x++) {
    for (size_t z = 0; z < block_depth; z++) {
      for (size_t y = 0; y < block_height; y++) {
        if (auto &block = chunk->get_block(x, y, z);
            block.type != block::Type::AIR) {
          block.generate(block_server, m_vertices, m_indices,
                         glm::vec3(static_cast<float>(x) + pos.x + 0.5f,
                                   static_cast<float>(y) + 0.0f + 0.5f,
                                   static_cast<float>(z) + pos.y + 0.5f));
        }
      }
    }
  }

#ifndef NDEBUG
  const auto end_time(std::chrono::high_resolution_clock::now());
  std::stringstream stream;
  stream << "Mesh VTime: "
         << std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                  start_time)
                .count()
         << " µs";
  ::core::Log::info(stream.str());
#endif
}

void Mesh::load_buffer() {
  const auto vertices_size{sizeof(Vertex) * m_vertices.size()};
  const auto indices_size{sizeof(uint32_t) * m_indices.size()};

  if (vertices_size == 0) {
    m_vertex_buffer.reset();
    m_index_buffer.reset();
  } else {
    if (!m_vertex_buffer) {
      m_vertex_buffer = std::make_unique<::core::vulkan::Buffer>(
          m_context, vk::BufferUsageFlagBits::eVertexBuffer, vertices_size);
    }

    if (!m_index_buffer) {
      m_index_buffer = std::make_unique<::core::vulkan::Buffer>(
          m_context, vk::BufferUsageFlagBits::eIndexBuffer, indices_size);
    }

    m_vertex_buffer->set_data(m_vertices.data(), vertices_size);
    m_index_buffer->set_data(m_indices.data(), indices_size);
    m_num_indices = m_indices.size();
  }

  m_vertices.clear();
  m_indices.clear();
}
}; // namespace chunk