#pragma once
#include "mesh.hpp"
#include <array>

namespace chunk {
class Chunk {
public:
  static constexpr size_t chunk_width = 32;
  static constexpr size_t chunk_depth = 32;
  static constexpr size_t chunk_height = 256;

  Chunk(const ::core::vulkan::Context &context, const glm::vec3 &position);

  void render(const ::core::vulkan::RenderCall &render_call);

private:
  std::array<std::array<std::array<bool, chunk_height>, chunk_depth>,
             chunk_width>
      m_blocks;

  Mesh m_mesh;
  const glm::vec3 m_position;
};
} // namespace chunk