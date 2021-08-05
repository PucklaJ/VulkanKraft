#include "world_generation.hpp"

namespace world_gen {
WorldGeneration::WorldGeneration(const size_t seed) { m_noise.seed(seed); }

void WorldGeneration::generate(const glm::ivec2 &chunk_pos,
                               chunk::BlockArray &block_array) const {
  for (size_t x = 0; x < chunk::block_width; x++) {
    for (size_t z = 0; z < chunk::block_depth; z++) {
      const glm::vec2 block_pos(
          static_cast<float>(chunk_pos.x) + static_cast<float>(x),
          static_cast<float>(chunk_pos.y) + static_cast<float>(z));

      auto noise_value{m_noise.get(block_pos.x, block_pos.y, 0.01f)};
      noise_value += 1.0f;
      noise_value /= 2.0f;
      noise_value *= static_cast<float>(chunk::block_height);

      const auto max_height{static_cast<size_t>(noise_value)};
      for (size_t y = 0; y < max_height; y++) {
        block_array.set(x, y, z, block::Type::GRASS);
      }
      for (size_t y = max_height; y < chunk::block_height; y++) {
        block_array.set(x, y, z, block::Type::AIR);
      }
    }
  }
}
} // namespace world_gen
