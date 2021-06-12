#pragma once
#include "../core/math.hpp"
#include "mesh.hpp"
#include <array>
#include <memory>

namespace chunk {
class Chunk : public BlockArray {
public:
  friend class Mesh;

  Chunk(const ::core::vulkan::Context &context, const glm::ivec2 &position);

  void generate();
  void generate_block_change(const glm::ivec3 &position);
  ::core::math::AABB to_aabb() const;
  void update_faces();
  void render(const ::core::vulkan::RenderCall &render_call);

  inline void set_front(std::shared_ptr<Chunk> c) { m_front = c; }
  inline void set_back(std::shared_ptr<Chunk> c) { m_back = c; }
  inline void set_left(std::shared_ptr<Chunk> c) { m_left = c; }
  inline void set_right(std::shared_ptr<Chunk> c) { m_right = c; }

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

private:
  static void _check_faces(const Chunk *chunk, const size_t x, const size_t y,
                           const size_t z, bool &front_face, bool &back_face,
                           bool &right_face, bool &left_face, bool &top_face,
                           bool &bot_face);

  void _check_faces_of_block(const glm::ivec3 &position);
  void _check_neighboring_faces_of_block(const glm::ivec3 &position);

  Mesh m_mesh;
  const glm::ivec2 m_position;
  bool m_first_generated;

  std::weak_ptr<Chunk> m_front;
  std::weak_ptr<Chunk> m_back;
  std::weak_ptr<Chunk> m_left;
  std::weak_ptr<Chunk> m_right;
};
} // namespace chunk