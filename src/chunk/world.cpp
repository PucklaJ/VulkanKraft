#include "world.hpp"
#include "../core/exception.hpp"
#include "../core/log.hpp"
#include <sstream>

namespace chunk {
World::World(const ::core::vulkan::Context &context, const size_t width,
             const size_t depth) {
  for (size_t x = 0; x < width; x++) {
    for (size_t z = 0; z < depth; z++) {
      const glm::ivec2 pos(x * block_width, z * block_depth);
      auto chunk = std::make_shared<Chunk>(context, pos);
      if (x != 0) {
        auto left = m_chunks[std::make_pair(static_cast<int>(x - 1), static_cast<int>(z))];
        left->set_right(chunk);
        chunk->set_left(left);
      }
      if (z != 0) {
        auto front = m_chunks[std::make_pair(static_cast<int>(x), static_cast<int>(z - 1))];
        front->set_back(chunk);
        chunk->set_front(front);
      }

      m_chunks.emplace(std::make_pair(static_cast<int>(x), static_cast<int>(z)), std::move(chunk));
    }
  }

  for (auto &[pos, chunk] : m_chunks) {
    chunk->generate();
  }
}

World::~World() {
  for (auto &[pos, chunk] : m_chunks) {
    chunk->destroy();
    chunk.reset();
  }
}

void World::place_block(const glm::ivec3 &position,
                        const BlockArray::BlockType block) {
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
                  chunk_block_position.z, false);
    (*chunk)->generate_block_change(chunk_block_position);
    return;
  }

  std::stringstream stream;
  stream << "attempted to destroy block at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

BlockArray::BlockType World::show_block(const glm::ivec3 &position) const {
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
