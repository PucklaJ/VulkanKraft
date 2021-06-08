#include "chunk.hpp"
#include <algorithm>

namespace chunk {
Chunk::Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position)
    : m_mesh(context), m_position(position) {
  fill();
}

void Chunk::generate() { m_mesh.generate(this, m_position); }

void Chunk::render(const ::core::vulkan::RenderCall &render_call) {
  m_mesh.render(render_call);
}
} // namespace chunk
