#pragma once
#include "chunk.hpp"
#include <map>
#include <mutex>
#include <optional>
#include <thread>

namespace chunk {
class World {
public:
  World(const ::core::vulkan::Context &context);
  ~World();

  void place_block(const glm::ivec3 &position, const BlockType block);
  void destroy_block(const glm::ivec3 &position);
  BlockType show_block(const glm::ivec3 &position);
  std::optional<glm::ivec3> raycast_block(const ::core::math::Ray &ray,
                                          ::core::math::Ray::Face &face);

  void render(const ::core::vulkan::RenderCall &render_call);
  void start_update_thread();

  inline void set_center_position(const glm::vec3 &pos) {
    std::lock_guard lk(m_center_position_mutex);
    m_center_position = pos;
  }

  inline void set_render_distance(const int render_distance) {
    m_render_distance = render_distance;
  }

private:
  static constexpr int raycast_distance = 10;
  static constexpr size_t update_wait_fps = 100;

  static inline std::pair<int, int>
  get_chunk_position(const glm::ivec3 &block_position) {
    return get_chunk_position(glm::ivec2(block_position.x, block_position.z));
  }
  static inline std::pair<int, int>
  get_chunk_position(const glm::vec3 &block_position) {
    return get_chunk_position(glm::ivec3(block_position));
  }
  static inline std::pair<int, int>
  get_chunk_position(const glm::ivec2 &block_position) {
    std::pair chunk_pos(block_position.x / block_width,
                        block_position.y / block_depth);
    chunk_pos.first -= 1 * (block_position.x < 0 &&
                            chunk_pos.first * block_width != block_position.x);
    chunk_pos.second -=
        1 * (block_position.y < 0 &&
             chunk_pos.second * block_depth != block_position.y);
    return chunk_pos;
  }
  static inline glm::ivec2 get_world_position(const std::pair<int, int> &pos) {
    return glm::ivec2(pos.first * block_width, pos.second * block_depth);
  }

  static bool _chunks_to_update_contains(
      const std::vector<std::weak_ptr<Chunk>> &chunks_to_update,
      std::shared_ptr<Chunk> chunk);

  std::optional<std::shared_ptr<Chunk>>
  _get_chunk(const std::pair<int, int> &pos);
  const std::optional<const std::shared_ptr<Chunk>>
  _get_chunk(const std::pair<int, int> &pos) const;
  std::shared_ptr<Chunk>
  _check_if_add_neighbour(std::pair<int, int> pos, std::shared_ptr<Chunk> chunk,
                          const std::pair<int, int> &center_position,
                          const int x, const int y);
  void _update();

  std::map<std::pair<int, int>, std::shared_ptr<Chunk>> m_chunks;
  std::unique_ptr<std::thread> m_chunk_update_thread;
  std::atomic<int> m_render_distance;
  glm::vec3 m_center_position;
  std::atomic<bool> m_running;
  std::vector<std::shared_ptr<Chunk>> m_chunks_to_delete;

  std::mutex m_chunks_mutex;
  std::mutex m_center_position_mutex;

  std::map<std::pair<int, int>,
           std::array<uint8_t, block_width * block_height * block_depth>>
      m_stored_blocks;

  const ::core::vulkan::Context &m_context;
};
} // namespace chunk
