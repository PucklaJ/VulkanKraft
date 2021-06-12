#include "world.hpp"
#include "../core/exception.hpp"
#include "../core/log.hpp"
#include <limits>
#include <set>
#include <sstream>

namespace chunk {
World::World(const ::core::vulkan::Context &context, const int width,
             const int depth) {
  for (int x = 0; x < width / block_width; x++) {
    for (int z = 0; z < depth / block_depth; z++) {
      const glm::ivec2 pos(x * block_width, z * block_depth);
      auto chunk = std::make_shared<Chunk>(context, pos);
      if (x != 0) {
        auto left = m_chunks[std::make_pair(x - 1, z)];
        left->set_right(chunk);
        chunk->set_left(left);
      }
      if (z != 0) {
        auto front = m_chunks[std::make_pair(x, z - 1)];
        front->set_back(chunk);
        chunk->set_front(front);
      }

      m_chunks.emplace(std::make_pair(x, z), std::move(chunk));
    }
  }

  for (auto &[pos, chunk] : m_chunks) {
    chunk->generate();
  }
}

void World::place_block(const glm::ivec3 &position, const BlockType block) {
  {
    std::stringstream stream;
    stream << "Placing block " << std::boolalpha << block << " at ";
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
World::raycast_block(const ::core::math::Ray &ray) const {
  // Get chunk of ray
  const std::pair chunk_pos(static_cast<int>(ray.origin.x) / block_width,
                            static_cast<int>(ray.origin.z) / block_depth);

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
  chunk_world_pos.y = 0.0f;

  for (const auto &c : ray_chunks) {
    if (!c)
      continue;

    const auto aabb(c->to_aabb());
    if (ray.cast(aabb) >= 0.0f) {
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

              const auto t{ray.cast(b.to_aabb(block_pos))};

              if (t >= 0.0f && t < t_min) {
                t_min = t;

                block_world_pos.x = static_cast<int>(block_pos.x);
                block_world_pos.y = y;
                block_world_pos.z = static_cast<int>(block_pos.z);
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

void World::render(const ::core::vulkan::RenderCall &render_call) {
  for (auto &[pos, chunk] : m_chunks) {
    chunk->render(render_call);
  }
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
} // namespace chunk
