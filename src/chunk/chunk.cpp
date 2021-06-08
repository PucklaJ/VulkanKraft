#include "chunk.hpp"
#include <algorithm>

namespace chunk {
Chunk::Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position)
    : m_mesh(context), m_position(position) {
  fill();
}

void Chunk::generate() { m_mesh.generate(this, m_position); }

void Chunk::generate_block_change(const glm::ivec3 &position) {
  if (position.x == 0 && m_left) {
    m_left->generate();
  }
  if (position.x == block_width - 1 && m_right) {
    m_right->generate();
  }
  if (position.z == 0 && m_front) {
    m_front->generate();
  }
  if (position.z == block_depth - 1 && m_back) {
    m_back->generate();
  }

  generate();
}

void Chunk::render(const ::core::vulkan::RenderCall &render_call) {
  m_mesh.render(render_call);
}

void Chunk::destroy() {
  if (m_front)
    m_front->m_back.reset();
  if (m_back)
    m_back->m_front.reset();
  if (m_left)
    m_left->m_right.reset();
  if (m_right)
    m_right->m_left.reset();
}
} // namespace chunk
