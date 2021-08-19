#pragma once
#include "../physics/aabb.hpp"
#include "chunk.hpp"
#include <map>
#include <mutex>
#include <optional>
#include <thread>

namespace physics {
class Server;
}

namespace chunk {
// This represents the whole voxel world consisting of chunks
class World {
public:
  friend class physics::Server;

  World(const ::core::vulkan::Context &context,
        const block::Server &block_server);
  ~World();

  // Place a block of the given type at the given world position
  void place_block(const glm::ivec3 &position, const block::Type block);
  // Destroy the block at the given world position
  void destroy_block(const glm::ivec3 &position);
  // Returns what type of block is at the given world position
  block::Type show_block(const glm::ivec3 &position);
  // Cast a ray and return the world position at which the ray is hitting a
  // block face .... which face of the block is hit by the ray
  std::optional<glm::ivec3> raycast_block(const ::core::math::Ray &ray,
                                          ::core::math::Ray::Face &face);

  // Render out the chunks
  void render(const ::core::vulkan::RenderCall &render_call);
  // Start the background thread which will generate new chunks and destroy
  // chunks which are too far away
  void start_update_thread();
  // Wait until there have been chunk_count chunks generated
  void wait_for_generation(const size_t chunk_count);
  // Returns the highest block position at the given world position. Only uses x
  // and z
  std::optional<int> get_height(const glm::vec3 &position);
  // Clears all chunks and generates a new world with a different seed. Used for
  // debugging
  void clear_and_reseed();

  // Set the position at which the player resides
  inline void set_center_position(const glm::vec3 &pos) {
    std::lock_guard lk(m_center_position_mutex);
    m_center_position = pos;
  }

  // returns the fog max distance
  inline float set_render_distance(const int render_distance) {
    m_render_distance = render_distance;
    return static_cast<float>(render_distance) *
           static_cast<float>(block_width);
  }

private:
  // Determines how far away from a ray a block can be in number of blocks
  static constexpr int raycast_distance = 10;
  // Sets the max fps of the backround update thread
  static constexpr size_t update_wait_fps = 100;
  // Sets the wait time for the wait_for_generation method
  static constexpr size_t generation_wait_fps = update_wait_fps / 4;

  // Converts a world position into a chunk position which can be used to
  // retrieve chunks from the m_chunks map. Use only one of these methods if you
  // want to convert a world position to a chunk position
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
  // Converts a chunk position into a world position
  static inline glm::ivec2 get_world_position(const std::pair<int, int> &pos) {
    return glm::ivec2(pos.first * block_width, pos.second * block_depth);
  }

  // Returns wether chunk is contained in chunks_to_update
  // Returns true if chunk is nullptr
  // Returns false if chunks_to_update is empty or holds only nullptr values
  static bool _chunks_to_update_contains(
      const std::vector<std::weak_ptr<Chunk>> &chunks_to_update,
      std::shared_ptr<Chunk> chunk);

  // Returns a chunk (if it exists) at the given chunk position
  std::optional<std::shared_ptr<Chunk>>
  _get_chunk(const std::pair<int, int> &pos);
  const std::optional<const std::shared_ptr<Chunk>>
  _get_chunk(const std::pair<int, int> &pos) const;
  // Returns a newly created chunk if it needs to be created
  // pos .................... the chunk position of the chunk to which should be
  // a neighbor added. chunk .................. the chunk to which a neighbor
  // should be added. center_position ........ the position from which world
  // will be viewed
  // x,y ...... the relative chunk position to chunk where a new chunk should be
  // created
  std::shared_ptr<Chunk>
  _check_if_add_neighbour(std::pair<int, int> pos, std::shared_ptr<Chunk> chunk,
                          const std::pair<int, int> &center_position,
                          const int x, const int y);
  // The background update thread function
  void _update();

  // Stores all chunks that are currently rendered
  std::map<std::pair<int, int>, std::shared_ptr<Chunk>> m_chunks;
  // The background update thread
  std::unique_ptr<std::thread> m_chunk_update_thread;
  // The maximum distance at which chunks are visible in number of chunks
  std::atomic<int> m_render_distance;
  // The position from which the world is viewed
  glm::vec3 m_center_position;
  // If the background update thread should be running
  std::atomic<bool> m_running;
  // Stores which chunks should be deleted
  // This vector will be populated by the background update thread and the
  // chunks will be deleted in the main thread
  std::vector<std::shared_ptr<Chunk>> m_chunks_to_delete;

  // A mutex which locks all access directly to the m_chunks map
  std::mutex m_chunks_mutex;
  // A mutex which locks all access to m_center_position
  std::mutex m_center_position_mutex;

  // Stores all blocks of the chunks which are currently not getting rendered
  std::map<std::pair<int, int>,
           std::array<uint8_t, block_width * block_height * block_depth>>
      m_stored_blocks;

  // Used to generate a procedural terrain
  world_gen::WorldGeneration m_world_generation;

  const ::core::vulkan::Context &m_context;
  const block::Server &m_block_server;
};
} // namespace chunk
