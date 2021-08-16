#include "server.hpp"

namespace block {
Server::Server() {

  // tex_coords.x ...... left
  // tex_coords.y ...... top
  // tex_coords.z ...... right
  // tex_coords.w ...... bot

  // Grass
  {
    BlockData data;
    data.tex_coords.front = _block_tex_coords(1, 2);
    data.tex_coords.back = _block_tex_coords(1, 0);
    data.tex_coords.left = _block_tex_coords(0, 1);
    data.tex_coords.right = _block_tex_coords(2, 1);
    data.tex_coords.bot = _block_tex_coords(1, 3);
    data.tex_coords.top = _block_tex_coords(1, 1);

    m_block_data.emplace(Type::GRASS, std::move(data));
  }

  // Dirt
  {
    BlockData data;

    data.tex_coords.front = _block_tex_coords(4, 2);
    data.tex_coords.back = _block_tex_coords(4, 2);
    data.tex_coords.left = _block_tex_coords(4, 2);
    data.tex_coords.right = _block_tex_coords(4, 2);
    data.tex_coords.bot = _block_tex_coords(4, 2);
    data.tex_coords.top = _block_tex_coords(4, 2);

    m_block_data.emplace(Type::DIRT, std::move(data));
  }
}

} // namespace block
