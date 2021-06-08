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

  void place_block(const glm::ivec3 &position,
                   const BlockArray::BlockType block);
  void destroy_block(const glm::ivec3 &position);
  BlockArray::BlockType show_block(const glm::ivec3 &position) const;

  void render(const ::core::vulkan::RenderCall &render_call);

private:
  static inline std::pair<int, int>
  get_chunk_position(const glm::ivec3 &block_position) {
    return std::pair(block_position.x / block_width,
                     block_position.z / block_depth);
  }

  std::optional<std::shared_ptr<Chunk>>
  _get_chunk(const std::pair<int, int> &pos);
  const std::optional<const std::shared_ptr<Chunk>>
  _get_chunk(const std::pair<int, int> &pos) const;

  std::map<std::pair<int, int>, std::shared_ptr<Chunk>> m_chunks;
};
} // namespace chunk
