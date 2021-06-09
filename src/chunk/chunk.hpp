#pragma once
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
  void render(const ::core::vulkan::RenderCall &render_call);

  inline void set_front(std::shared_ptr<Chunk> c) { m_front = c; }
  inline void set_back(std::shared_ptr<Chunk> c) { m_back = c; }
  inline void set_left(std::shared_ptr<Chunk> c) { m_left = c; }
  inline void set_right(std::shared_ptr<Chunk> c) { m_right = c; }
  inline const glm::ivec2 &get_position() const { return m_position; }

private:
  static void _check_faces_of_block(const Chunk *chunk, const size_t x,
                                    const size_t y, const size_t z,
                                    bool &front_face, bool &back_face,
                                    bool &right_face, bool &left_face,
                                    bool &top_face, bool &bot_face);

  Mesh m_mesh;
  const glm::ivec2 m_position;
  bool m_first_generated;

  std::weak_ptr<Chunk> m_front;
  std::weak_ptr<Chunk> m_back;
  std::weak_ptr<Chunk> m_left;
  std::weak_ptr<Chunk> m_right;
};
} // namespace chunk