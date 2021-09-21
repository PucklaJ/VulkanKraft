#pragma once
#include "../physics/aabb.hpp"
#include "../world_gen/world_generation.hpp"
#include "mesh.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <thread>

namespace chunk {
class Chunk : public BlockArray {
public:
  friend class Mesh;

  Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position);
  ~Chunk();

  void generate(const block::Server &block_server,
                const bool multi_thread = true);
  void generate_block_change(const block::Server &block_server,
                             const glm::ivec3 &position);
  physics::AABB to_aabb() const;
  void update_faces();
  void render(const ::core::vulkan::RenderCall &render_call);
  int get_height(glm::ivec3 world_pos) const;

  inline void set_front(std::shared_ptr<Chunk> c) { m_front = c; }
  inline void set_back(std::shared_ptr<Chunk> c) { m_back = c; }
  inline void set_left(std::shared_ptr<Chunk> c) { m_left = c; }
  inline void set_right(std::shared_ptr<Chunk> c) { m_right = c; }
  inline void needs_face_update() { m_needs_face_update = true; }

  inline std::shared_ptr<Chunk> get_front() { return m_front.lock(); }
  inline std::shared_ptr<Chunk> get_back() { return m_back.lock(); }
  inline std::shared_ptr<Chunk> get_left() { return m_left.lock(); }
  inline std::shared_ptr<Chunk> get_right() { return m_right.lock(); }
  inline const std::shared_ptr<Chunk> get_front() const {
    return m_front.lock();
  }
  inline const std::shared_ptr<Chunk> get_back() const { return m_back.lock(); }
  inline const std::shared_ptr<Chunk> get_left() const { return m_left.lock(); }
  inline const std::shared_ptr<Chunk> get_right() const {
    return m_right.lock();
  }
  inline const glm::ivec2 &get_position() const { return m_position; }

  void
  from_world_generation(const world_gen::WorldGeneration &world_generation);

private:
  static void _check_faces(const Chunk *chunk, const size_t x, const size_t y,
                           const size_t z, Block &block);

  void _check_faces_of_block(const glm::ivec3 &position);
  void _check_neighboring_faces_of_block(const glm::ivec3 &position);

  Mesh m_mesh;
  const glm::ivec2 m_position;
  std::atomic<bool> m_needs_face_update;
  std::unique_ptr<std::thread> m_generate_thread;
  // Wether there are currently newly generated vertices that need to be
  // uploaded to the GPU
  std::atomic<bool> m_vertices_ready;
  // Wether the generate thread has been started, but has not been joined yet
  std::atomic<bool> m_generating;

  std::weak_ptr<Chunk> m_front;
  std::weak_ptr<Chunk> m_back;
  std::weak_ptr<Chunk> m_left;
  std::weak_ptr<Chunk> m_right;
};
} // namespace chunk