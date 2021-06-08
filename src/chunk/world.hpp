#pragma once
#include "chunk.hpp"
#include <map>
#include <optional>

namespace chunk {
class World {
public:
  World(const ::core::vulkan::Context &context, const size_t width,
        const size_t depth);
  ~World();

  void render(const ::core::vulkan::RenderCall &render_call);

private:
  std::optional<std::shared_ptr<Chunk>> _get_chunk(const glm::ivec2 &pos);

  std::map<std::pair<int, int>, std::shared_ptr<Chunk>> m_chunks;
};
} // namespace chunk
