#include "chunk.hpp"
#include <algorithm>

namespace chunk {
Chunk::Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position)
    : m_mesh(context), m_position(position) {
  fill();
}

void Chunk::generate() { m_mesh.generate(this, m_position); }

void Chunk::generate_block_change(const glm::ivec3 &position) {
  if (auto left(m_left.lock()); position.x == 0 && left) {
    left->generate();
  }
  if (auto right(m_right.lock()); position.x == block_width - 1 && right) {
    right->generate();
  }
  if (auto front(m_front.lock()); position.z == 0 && front) {
    front->generate();
  }
  if (auto back(m_back.lock()); position.z == block_depth - 1 && back) {
    back->generate();
  }

  generate();
}

void Chunk::render(const ::core::vulkan::RenderCall &render_call) {
  m_mesh.render(render_call);
}

} // namespace chunk
