#include "world.hpp"

namespace chunk {
World::World(const ::core::vulkan::Context &context, const size_t width,
             const size_t depth) {
  for (size_t x = 0; x < width; x++) {
    for (size_t z = 0; z < depth; z++) {
      const glm::ivec2 pos(x * block_width, z * block_depth);
      auto chunk = std::make_shared<Chunk>(context, pos);
      if (x != 0) {
        auto left = m_chunks[std::make_pair<int, int>(x - 1, z)];
        left->set_right(chunk);
        chunk->set_left(left);
      }
      if (z != 0) {
        auto front = m_chunks[std::make_pair<int, int>(x, z - 1)];
        front->set_back(chunk);
        chunk->set_front(front);
      }

      m_chunks.emplace(std::make_pair<int, int>(x, z), std::move(chunk));
    }
  }

  for (auto &[pos, chunk] : m_chunks) {
    chunk->generate();
  }
}

World::~World() {
  for (auto &[pos, chunk] : m_chunks) {
    chunk->destroy();
    chunk.reset();
  }
}

void World::render(const ::core::vulkan::RenderCall &render_call) {
  for (auto &[pos, chunk] : m_chunks) {
    chunk->render(render_call);
  }
}

std::optional<std::shared_ptr<Chunk>> World::_get_chunk(const glm::ivec2 &pos) {
  const auto p(std::make_pair(pos.x, pos.y));
  if (m_chunks.find(p) == m_chunks.end()) {
    return std::nullopt;
  }

  return m_chunks[p];
}
} // namespace chunk
