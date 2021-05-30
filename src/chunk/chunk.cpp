#include "chunk.hpp"
#include <algorithm>

namespace chunk {
Chunk::Chunk(const ::core::vulkan::Context &context, const glm::vec3 &position)
    : m_mesh(context), m_position(position) {
  for (size_t x = 0; x < chunk_width; x++) {
    for (size_t z = 0; z < chunk_depth; z++) {
      for (size_t y = 0; y < chunk_height; y++) {
        m_blocks[x][z][y] = true;
      }
    }
  }

  m_mesh.generate(m_blocks, m_position);
}

void Chunk::render(const ::core::vulkan::RenderCall &render_call) {
  m_mesh.render(render_call);
}
} // namespace chunk
