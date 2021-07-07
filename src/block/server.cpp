#include "server.hpp"

namespace block {
Server::Server() {
  // Grass
  {
    BlockData data;
    data.tex_coords.front = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.tex_coords.back = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.tex_coords.left = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.tex_coords.right = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.tex_coords.bot = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.tex_coords.top = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    m_block_data.emplace(Type::GRASS, std::move(data));
  }
}

} // namespace block
