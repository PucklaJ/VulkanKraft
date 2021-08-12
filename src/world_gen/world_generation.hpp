#pragma once

#include "../chunk/block.hpp"
#include "perlin_noise.hpp"

namespace world_gen {
class WorldGeneration {
public:
  WorldGeneration(const size_t seed_value = 0);

  void seed(const size_t seed_value);

  void generate(const glm::ivec2 &chunk_pos,
                chunk::BlockArray &block_array) const;

private:
  PerlinNoise m_noise;
};
} // namespace world_gen
