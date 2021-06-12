#include "world.hpp"
#include "../core/exception.hpp"
#include "../core/log.hpp"
#include <chrono>
#include <limits>
#include <set>
#include <sstream>

namespace chunk {
World::World(const ::core::vulkan::Context &context) : m_context(context) {}

void World::place_block(const glm::ivec3 &position, const BlockType block) {
  {
    std::stringstream stream;
    stream << "Placing block " << std::boolalpha << block << " at ";
    stream << "(" << position.x << "; " << position.y << "; " << position.z
           << ")";
    ::core::Log::info(stream.str());
  }

  if (position.y >= block_height) {
    throw core::VulkanKraftException(std::to_string(position.y) +
                                     " is above height limit");
  }

  const auto chunk_pos(get_chunk_position(position));

  auto chunk(_get_chunk(chunk_pos));
  if (chunk) {
    const glm::ivec3 chunk_block_position(
        position.x - (*chunk)->get_position().x, position.y,
        position.z - (*chunk)->get_position().y);

    (*chunk)->set(chunk_block_position.x, chunk_block_position.y,
                  chunk_block_position.z, block);
    (*chunk)->generate_block_change(chunk_block_position);
    return;
  }

  std::stringstream stream;
  stream << "attempted to place block " << std::boolalpha << block
         << " at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

void World::destroy_block(const glm::ivec3 &position) {
  {
    std::stringstream stream;
    stream << "Destroying block  at ";
    stream << "(" << position.x << "; " << position.y << "; " << position.z
           << ")";
    ::core::Log::info(stream.str());
  }

  const auto chunk_pos(get_chunk_position(position));

  auto chunk(_get_chunk(chunk_pos));
  if (chunk) {
    const glm::ivec3 chunk_block_position(
        position.x - (*chunk)->get_position().x, position.y,
        position.z - (*chunk)->get_position().y);

    (*chunk)->set(chunk_block_position.x, chunk_block_position.y,
                  chunk_block_position.z, BlockType::AIR);
    (*chunk)->generate_block_change(chunk_block_position);
    return;
  }

  std::stringstream stream;
  stream << "attempted to destroy block at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

BlockType World::show_block(const glm::ivec3 &position) const {
  const auto chunk_pos(get_chunk_position(position));

  const auto chunk(_get_chunk(chunk_pos));
  if (chunk) {
    const glm::ivec3 chunk_block_position(
        position.x - (*chunk)->get_position().x, position.y,
        position.z - (*chunk)->get_position().y);

    return (*chunk)->get(chunk_block_position.x, chunk_block_position.y,
                         chunk_block_position.z);
  }

  std::stringstream stream;
  stream << "attempted to show block at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

std::optional<glm::ivec3>
World::raycast_block(const ::core::math::Ray &ray,
                     ::core::math::Ray::Face &face) const {
  // Get chunk of ray
  const auto chunk_pos(get_chunk_position(ray.origin));

  const auto _ray_chunk = _get_chunk(chunk_pos);
  if (!_ray_chunk) {
    return std::nullopt;
  }
  const auto ray_chunk = *_ray_chunk;

  // Raycast ray chunk and neighbouring
  std::set<std::shared_ptr<Chunk>> ray_chunks = {
      ray_chunk, ray_chunk->get_front(), ray_chunk->get_back(),
      ray_chunk->get_left(), ray_chunk->get_right()};
  if (auto left = ray_chunk->get_left(); left) {
    ray_chunks.emplace(left->get_front());
    ray_chunks.emplace(left->get_back());
  }
  if (auto right = ray_chunk->get_right(); right) {
    ray_chunks.emplace(right->get_front());
    ray_chunks.emplace(right->get_back());
  }

  auto t_min{std::numeric_limits<float>::max()};
  glm::vec3 chunk_world_pos;
  glm::ivec3 block_world_pos;
  ::core::math::Ray::Face ray_face;

  chunk_world_pos.y = 0.0f;

  for (const auto &c : ray_chunks) {
    if (!c)
      continue;

    const auto aabb(c->to_aabb());
    if (ray.cast(aabb, ray_face) >= 0.0f) {
      chunk_world_pos.x = static_cast<float>(c->get_position().x);
      chunk_world_pos.z = static_cast<float>(c->get_position().y);

      // Loop over all blocks of chunk
      for (int x = 0; x < block_width; x++) {
        for (int y = 0; y < block_height; y++) {
          for (int z = 0; z < block_depth; z++) {
            const auto &b = c->get_block(x, y, z);
            if (b.type != BlockType::AIR) {
              const glm::vec3 block_pos(
                  chunk_world_pos.x + static_cast<float>(x),
                  static_cast<float>(y),
                  chunk_world_pos.z + static_cast<float>(z));

              if (abs(ray.origin.x - block_pos.x) > raycast_distance ||
                  abs(ray.origin.y - block_pos.y) > raycast_distance ||
                  abs(ray.origin.z - block_pos.z) > raycast_distance) {
                continue;
              }

              const auto t{ray.cast(b.to_aabb(block_pos), ray_face)};

              if (t >= 0.0f && t < t_min) {
                t_min = t;

                block_world_pos.x = static_cast<int>(block_pos.x);
                block_world_pos.y = y;
                block_world_pos.z = static_cast<int>(block_pos.z);
                face = ray_face;
              }
            }
          }
        }
      }
    }
  }

  // If we intersected with any block
  if (t_min != std::numeric_limits<float>::max()) {
    return block_world_pos;
  }

  return std::nullopt;
}

void World::update(const glm::vec3 &_center_position,
                   const int render_distance) {
  const auto center_position(get_chunk_position(_center_position));
  // First remove all chunks that are too far away
  std::set<std::pair<int, int>> chunks_to_remove;
  for (const auto &[pos, chunk] : m_chunks) {
    if (abs(pos.first - center_position.first) > render_distance ||
        abs(pos.second - center_position.second) > render_distance) {
      chunks_to_remove.emplace(pos);
    }
  }

  if (!chunks_to_remove.empty()) {
#ifndef NDEBUG
    const auto start_time = std::chrono::high_resolution_clock::now();
#endif
    for (const auto &pos : chunks_to_remove) {
      auto &chunk = m_chunks[pos];
      m_chunks.erase(pos);
    }
#ifndef NDEBUG
    const auto end_time = std::chrono::high_resolution_clock::now();
    std::stringstream stream;
    stream << "Chunk Remove Time: "
           << std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                    start_time)
                  .count()
           << " µs";
    ::core::Log::info(stream.str());
#endif
  }

  std::vector<std::weak_ptr<Chunk>> chunks_to_update;
  if (m_chunks.empty()) {
    auto chunk =
        std::make_shared<Chunk>(m_context, get_world_position(center_position));
    m_chunks.emplace(center_position, chunk);
    chunks_to_update.emplace_back(chunk);
  }

#ifndef NDEBUG
  const auto add_start_time = std::chrono::high_resolution_clock::now();
#endif

  // Loop over all chunks and check if neighbors should be added
  std::vector<std::shared_ptr<Chunk>> chunks_to_add;
  for (const auto &[pos, chunk] : m_chunks) {
    if (!chunk->get_right()) {
      // right
      chunks_to_add.emplace_back(_check_if_add_neighbour(
          pos, chunk, center_position, 1, 0, render_distance));
    }
    if (!chunk->get_left()) {
      // left
      chunks_to_add.emplace_back(_check_if_add_neighbour(
          pos, chunk, center_position, -1, 0, render_distance));
    }
    if (!chunk->get_front()) {
      // front
      chunks_to_add.emplace_back(_check_if_add_neighbour(
          pos, chunk, center_position, 0, -1, render_distance));
    }
    if (!chunk->get_back()) {
      // back
      chunks_to_add.emplace_back(_check_if_add_neighbour(
          pos, chunk, center_position, 0, 1, render_distance));
    }
  }

  for (auto &chunk : chunks_to_add) {
    if (!chunk)
      continue;
    const auto chunk_pos(get_chunk_position(chunk->get_position()));
    m_chunks.emplace(get_chunk_position(chunk->get_position()), chunk);
    chunks_to_update.emplace_back(chunk);
    // if (!_chunks_to_update_contains(chunks_to_update, chunk->get_front()))
    //   chunks_to_update.emplace_back(chunk->get_front());
    // if (!_chunks_to_update_contains(chunks_to_update, chunk->get_back()))
    //   chunks_to_update.emplace_back(chunk->get_back());
    // if (!_chunks_to_update_contains(chunks_to_update, chunk->get_left()))
    //   chunks_to_update.emplace_back(chunk->get_left());
    // if (!_chunks_to_update_contains(chunks_to_update, chunk->get_right()))
    //   chunks_to_update.emplace_back(chunk->get_right());
  }
#ifndef NDEBUG
  if (!chunks_to_add.empty()) {
    const auto add_end_time = std::chrono::high_resolution_clock::now();
    {
      std::stringstream stream;
      stream << "Add Chunk Time: "
             << std::chrono::duration_cast<std::chrono::microseconds>(
                    add_end_time - add_start_time)
                    .count()
             << " µs";
      ::core::Log::info(stream.str());
    }
  }

  if (!chunks_to_update.empty()) {
    const auto gen_start_time = std::chrono::high_resolution_clock::now();
#endif

    chunks_to_add.clear();

    std::vector<std::thread> update_threads;
    // Update neighbouring chunks
    for (auto &_chunk : chunks_to_update) {
      if (auto chunk = _chunk.lock(); chunk) {
        update_threads.emplace_back([chunk]() {
          chunk->update_faces();
          chunk->generate();
        });
      }
    }
    for (auto &t : update_threads) {
      t.join();
    }
#ifndef NDEBUG
    const auto gen_end_time = std::chrono::high_resolution_clock::now();
    {
      std::stringstream stream;
      stream << "Gen Chunk Time: "
             << std::chrono::duration_cast<std::chrono::microseconds>(
                    gen_end_time - gen_start_time)
                    .count()
             << " µs";
      ::core::Log::info(stream.str());
    }
  }
#endif
}

void World::render(const ::core::vulkan::RenderCall &render_call) {
  for (auto &[pos, chunk] : m_chunks) {
    chunk->render(render_call);
  }
}

bool World::_chunks_to_update_contains(
    const std::vector<std::weak_ptr<Chunk>> &chunks_to_update,
    std::shared_ptr<Chunk> chunk) {
  if (!chunk) {
    return true;
  }

  for (auto &_c : chunks_to_update) {
    if (auto c = _c.lock(); c) {
      if (c == chunk) {
        return true;
      }
    }
  }

  return false;
}

std::optional<std::shared_ptr<Chunk>>
World::_get_chunk(const std::pair<int, int> &pos) {
  if (m_chunks.find(pos) == m_chunks.end()) {
    return std::nullopt;
  }

  return m_chunks[pos];
}

const std::optional<const std::shared_ptr<Chunk>>
World::_get_chunk(const std::pair<int, int> &pos) const {
  if (m_chunks.find(pos) == m_chunks.end()) {
    return std::nullopt;
  }

  return m_chunks.at(pos);
}

std::shared_ptr<Chunk> World::_check_if_add_neighbour(
    std::pair<int, int> pos, std::shared_ptr<Chunk> chunk,
    const std::pair<int, int> &center_position, const int x, const int y,
    const int render_distance) {
  pos.first += x;
  pos.second += y;

  if (abs(pos.first - center_position.first) > render_distance ||
      abs(pos.second - center_position.second) > render_distance) {
    return nullptr;
  }

  auto new_chunk = std::make_shared<Chunk>(m_context, get_world_position(pos));

  // right
  if (x == 1 && y == 0) {
    chunk->set_right(new_chunk);
    new_chunk->set_left(chunk);
    if (auto front = chunk->get_front(); front) {
      if (auto right = front->get_right(); right) {
        right->set_back(new_chunk);
        new_chunk->set_front(right);
      }
    }
    if (auto back = chunk->get_back(); back) {
      if (auto right = back->get_right(); right) {
        right->set_front(new_chunk);
        new_chunk->set_back(right);
      }
    }
    // left
  } else if (x == -1 && y == 0) {
    chunk->set_left(new_chunk);
    new_chunk->set_right(chunk);
    if (auto front = chunk->get_front(); front) {
      if (auto left = front->get_left(); left) {
        left->set_back(new_chunk);
        new_chunk->set_front(left);
      }
    }
    if (auto back = chunk->get_back(); back) {
      if (auto left = back->get_left(); left) {
        left->set_front(new_chunk);
        new_chunk->set_back(left);
      }
    }
    // back
  } else if (x == 0 && y == 1) {
    chunk->set_back(new_chunk);
    new_chunk->set_front(chunk);
    if (auto right = chunk->get_right(); right) {
      if (auto back = right->get_back(); back) {
        back->set_left(new_chunk);
        new_chunk->set_right(back);
      }
    }
    if (auto left = chunk->get_left(); left) {
      if (auto back = left->get_back(); back) {
        back->set_right(new_chunk);
        new_chunk->set_left(back);
      }
    }
    // front
  } else if (x == 0 && y == -1) {
    chunk->set_front(new_chunk);
    new_chunk->set_back(chunk);
    if (auto right = chunk->get_right(); right) {
      if (auto front = right->get_front(); front) {
        front->set_left(new_chunk);
        new_chunk->set_right(front);
      }
    }
    if (auto left = chunk->get_left(); left) {
      if (auto front = left->get_front(); front) {
        front->set_right(new_chunk);
        new_chunk->set_left(front);
      }
    }
  }

  return new_chunk;
}
} // namespace chunk
