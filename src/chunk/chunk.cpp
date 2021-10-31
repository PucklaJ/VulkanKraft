#include "chunk.hpp"
#include <algorithm>
#ifndef NDEBUG
#include "../core/log.hpp"
#endif

namespace chunk {
Chunk::Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position)
    : m_mesh(context), m_position(position), m_needs_face_update(false),
      m_vertices_ready(false), m_generating(false) {}

Chunk::~Chunk() {
  if (m_generating) {
    m_generate_thread->join();
    m_generating = false;
    m_generate_thread.reset();
  }
}

void Chunk::generate(const block::Server &block_server,
                     const bool multi_thread) {
  if (m_generating) {
    m_generate_thread->join();
    m_generating = false;
    m_generate_thread.reset();
  }

  if (!multi_thread) {
    if (m_needs_face_update) {
      update_faces();
      m_needs_face_update = false;
    }
    m_mesh.generate_vertices(block_server, this, m_position);
    m_mesh.load_buffer();
    return;
  }

  m_generate_thread = std::make_unique<std::thread>([&]() {
    if (m_needs_face_update) {
      update_faces();
      m_needs_face_update = false;
    }

    m_mesh.generate_vertices(block_server, this, m_position);

    m_vertices_ready = true;
  });

  m_generating = true;
}

void Chunk::generate_block_change(const block::Server &block_server,
                                  const glm::ivec3 &position) {
  compute_sun_light();
  _check_neighboring_faces_of_block(position);

  if (auto left(m_left.lock()); position.x == 0 && left) {
    left->_check_faces_of_block(
        glm::ivec3(block_width - 1, position.y, position.z));
    left->generate(block_server, false);
  }
  if (auto right(m_right.lock()); position.x == block_width - 1 && right) {
    right->_check_faces_of_block(glm::ivec3(0, position.y, position.z));
    right->generate(block_server, false);
  }
  if (auto front(m_front.lock()); position.z == 0 && front) {
    front->_check_faces_of_block(
        glm::ivec3(position.x, position.y, block_depth - 1));
    front->generate(block_server, false);
  }
  if (auto back(m_back.lock()); position.z == block_depth - 1 && back) {
    back->_check_faces_of_block(glm::ivec3(position.x, position.y, 0));
    back->generate(block_server, false);
  }

  generate(block_server, false);
}

physics::AABB Chunk::to_aabb() const {
  return physics::AABB(
      static_cast<float>(m_position.x), 0.0f, static_cast<float>(m_position.y),
      static_cast<float>(block_width), static_cast<float>(block_height),
      static_cast<float>(block_depth));
}

void Chunk::update_faces() {
  for (size_t x = 0; x < block_width; x++) {
    for (size_t y = 0; y < block_height; y++) {
      for (size_t z = 0; z < block_depth; z++) {
        _check_faces(this, x, y, z, get_block(x, y, z));
      }
    }
  }
}

void Chunk::render(const ::core::vulkan::RenderCall &render_call,
                   size_t &max_chunk_gen) {
  check_mesh(max_chunk_gen);

  m_mesh.render(render_call);
}

int Chunk::get_height(glm::ivec3 world_pos) const {
  world_pos.x -= m_position.x;
  world_pos.z -= m_position.y;

  int height{0};
  block::Type block_type;

  for (int y = 0; y < block_height; y++) {
    block_type = get(world_pos.x, y, world_pos.z);
    height = (block_type != block::Type::AIR) * (y + 1) +
             (block_type == block::Type::AIR) * height;
  }

  return height;
}

void Chunk::compute_sun_light() {
  for (int x = 0; x < block_width; x++) {
    for (int z = 0; z < block_depth; z++) {
      float light_value{1.0f};

      for (int y = block_height - 1; y >= 0; y--) {
        auto &block = get_block(x, y, z);
        if (block.type != block::Type::AIR) {
          block.set_top_light(light_value);
          light_value = 1.0f / static_cast<float>(0b1111);
        }

        if (x != 0) {
          get_block(x - 1, y, z).set_right_light(light_value);
        } else {
          if (auto left(m_left.lock()); left) {
            left->get_block(block_width - 1, y, z).set_right_light(light_value);
          }
        }

        if (x != block_width - 1) {
          get_block(x + 1, y, z).set_left_light(light_value);
        } else {
          if (auto right(m_right.lock()); right) {
            right->get_block(0, y, z).set_left_light(light_value);
          }
        }

        if (z != 0) {
          get_block(x, y, z - 1).set_front_light(light_value);
        } else {
          if (auto front(m_front.lock()); front) {
            front->get_block(x, y, block_depth - 1)
                .set_front_light(light_value);
          }
        }

        if (z != block_depth - 1) {
          get_block(x, y, z + 1).set_back_light(light_value);
        } else {
          if (auto back(m_back.lock()); back) {
            back->get_block(x, y, 0).set_back_light(light_value);
          }
        }
      }
    }
  }
}

void Chunk::from_world_generation(
    const world_gen::WorldGeneration &world_generation) {
  world_generation.generate(m_position, *this);
}

void Chunk::_check_faces(const Chunk *chunk, const size_t x, const size_t y,
                         const size_t z, Block &block) {
  block.left_face(x == 0 || !chunk->get(x - 1, y, z));
  block.right_face(x == block_width - 1 || !chunk->get(x + 1, y, z));

  block.front_face(z == block_depth - 1 || !chunk->get(x, y, z + 1));
  block.back_face(z == 0 || !chunk->get(x, y, z - 1));

  block.top_face(y == block_height - 1 || !chunk->get(x, y + 1, z));
  block.bot_face(y == 0 || !chunk->get(x, y - 1, z));

  if (auto left(chunk->m_left.lock()); x == 0) {
    block.left_face(!left || !left->get(block_width - 1, y, z));
  }
  if (auto front(chunk->m_front.lock()); z == 0) {
    block.back_face(!front || !front->get(x, y, block_depth - 1));
  }
  if (auto right(chunk->m_right.lock()); x == block_width - 1) {
    block.right_face(!right || !right->get(0, y, z));
  }
  if (auto back(chunk->m_back.lock()); z == block_depth - 1) {
    block.front_face(!back || !back->get(x, y, 0));
  }
}

void Chunk::_check_faces_of_block(const glm::ivec3 &pos) {
  _check_faces(this, pos.x, pos.y, pos.z, get_block(pos.x, pos.y, pos.z));
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
