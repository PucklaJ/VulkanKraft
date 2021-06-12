#include "chunk.hpp"
#include <algorithm>

namespace chunk {
Chunk::Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position)
    : m_mesh(context), m_position(position), m_first_generated(true) {
  fill();
}

void Chunk::generate() {
  if (m_first_generated) {
    for (size_t x = 0; x < block_width; x++) {
      for (size_t y = 0; y < block_height; y++) {
        for (size_t z = 0; z < block_depth; z++) {
          auto &block = get_block(x, y, z);
          _check_faces(this, x, y, z, block.front_face(), block.back_face(),
                       block.right_face(), block.left_face(), block.top_face(),
                       block.bot_face());
        }
      }
    }

    m_first_generated = false;
  }
  m_mesh.generate(this, m_position);
}

void Chunk::generate_block_change(const glm::ivec3 &position) {
  _check_neighboring_faces_of_block(position);

  if (auto left(m_left.lock()); position.x == 0 && left) {
    left->_check_faces_of_block(
        glm::ivec3(block_width - 1, position.y, position.z));
    left->generate();
  }
  if (auto right(m_right.lock()); position.x == block_width - 1 && right) {
    right->_check_faces_of_block(glm::ivec3(0, position.y, position.z));
    right->generate();
  }
  if (auto front(m_front.lock()); position.z == 0 && front) {
    front->_check_faces_of_block(
        glm::ivec3(position.x, position.y, block_depth - 1));
    front->generate();
  }
  if (auto back(m_back.lock()); position.z == block_depth - 1 && back) {
    back->_check_faces_of_block(glm::ivec3(position.x, position.y, 0));
    back->generate();
  }

  generate();
}

::core::math::AABB Chunk::to_aabb() const {
  ::core::math::AABB aabb;

  aabb.min.x = static_cast<float>(m_position.x);
  aabb.min.y = 0.0f;
  aabb.min.z = static_cast<float>(m_position.y);

  aabb.max.x = aabb.min.x + static_cast<float>(block_width);
  aabb.max.y = static_cast<float>(block_height);
  aabb.max.z = aabb.min.z + static_cast<float>(block_depth);

  return aabb;
}

void Chunk::render(const ::core::vulkan::RenderCall &render_call) {
  m_mesh.render(render_call);
}

void Chunk::_check_faces(const Chunk *chunk, const size_t x, const size_t y,
                         const size_t z, bool &front_face, bool &back_face,
                         bool &right_face, bool &left_face, bool &top_face,
                         bool &bot_face) {
  left_face = x == 0 || !chunk->get(x - 1, y, z);
  right_face = x == block_width - 1 || !chunk->get(x + 1, y, z);

  front_face = z == block_depth - 1 || !chunk->get(x, y, z + 1);
  back_face = z == 0 || !chunk->get(x, y, z - 1);

  top_face = y == block_height - 1 || !chunk->get(x, y + 1, z);
  bot_face = y == 0 || !chunk->get(x, y - 1, z);

  if (auto left(chunk->m_left.lock()); x == 0 && left) {
    left_face = !left->get(block_width - 1, y, z);
  }
  if (auto front(chunk->m_front.lock()); z == 0 && front) {
    back_face = !front->get(x, y, block_depth - 1);
  }
  if (auto right(chunk->m_right.lock()); x == block_width - 1 && right) {
    right_face = !right->get(0, y, z);
  }
  if (auto back(chunk->m_back.lock()); z == block_depth - 1 && back) {
    front_face = !back->get(x, y, 0);
  }
}

void Chunk::_check_faces_of_block(const glm::ivec3 &pos) {
  auto &block = get_block(pos.x, pos.y, pos.z);
  _check_faces(this, pos.x, pos.y, pos.z, block.front_face(), block.back_face(),
               block.right_face(), block.left_face(), block.top_face(),
               block.bot_face());
}

void Chunk::_check_neighboring_faces_of_block(const glm::ivec3 &position) {
  for (int x = -1; x <= 1; x++) {
    for (int z = -1; z <= 1; z++) {
      for (int y = -1; y <= 1; y++) {
        const glm::ivec3 pos(position.x + x, position.y + y, position.z + z);
        if ((pos.x >= 0 && pos.y >= 0 && pos.z >= 0) &&
            (pos.x < block_width && pos.y < block_height &&
             pos.z < block_depth)) {
          _check_faces_of_block(pos);
        }
      }
    }
  }
}

} // namespace chunk
