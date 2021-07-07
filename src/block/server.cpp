#include "server.hpp"

namespace block {
Server::Server() {
  constexpr auto ps2 = 1.0f / 512.0f / 2.0f;

  // Grass
  {
    BlockData data;
    data.tex_coords.front = glm::vec4(
        0.0f + ps2, 0.0f + ps2, 16.0f / 512.0f - ps2, 16.0f / 512.0f - ps2);
    data.tex_coords.back = glm::vec4(
        0.0f + ps2, 0.0f + ps2, 16.0f / 512.0f - ps2, 16.0f / 512.0f - ps2);
    data.tex_coords.left = glm::vec4(
        0.0f + ps2, 0.0f + ps2, 16.0f / 512.0f - ps2, 16.0f / 512.0f - ps2);
    data.tex_coords.right = glm::vec4(
        0.0f + ps2, 0.0f + ps2, 16.0f / 512.0f - ps2, 16.0f / 512.0f - ps2);
    data.tex_coords.bot = glm::vec4(0.0f + ps2, 0.0f + ps2,
                                    16.0f / 512.0f - ps2, 16.0f / 512.0f - ps2);
    data.tex_coords.top = glm::vec4(0.0f + ps2, 0.0f + ps2,
                                    16.0f / 512.0f - ps2, 16.0f / 512.0f - ps2);

    m_block_data.emplace(Type::GRASS, std::move(data));
  }
}

} // namespace block
