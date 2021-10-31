#pragma once
#include "../block/server.hpp"
#include "../block/type.hpp"
#include "../physics/aabb.hpp"
#include <array>
#include <glm/glm.hpp>
#include <vector>

namespace chunk {

constexpr int block_width = 16;
constexpr int block_depth = 16;
constexpr int block_height = 128;

class Vertex {
public:
  Vertex(float x, float y, float z, float u, float v, float light);

  glm::vec3 position;
  glm::vec2 uv;
  float light;
};
struct GlobalUniform {
  glm::mat4 proj_view;
  glm::vec3 eye_pos;
};

class Block {
public:
  static constexpr size_t default_face_count =
      (block_width * block_depth) * 2 + (block_height * block_width) * 2 +
      (block_height * block_depth) * 2;
  static constexpr size_t vertices_per_face = 4;
  static constexpr size_t indices_per_face = 6;

  Block();

  constexpr bool front_face() const { return _get_face(front_face_bit); }
  constexpr bool back_face() const { return _get_face(back_face_bit); }
  constexpr bool left_face() const { return _get_face(left_face_bit); }
  constexpr bool right_face() const { return _get_face(right_face_bit); }
  constexpr bool top_face() const { return _get_face(top_face_bit); }
  constexpr bool bot_face() const { return _get_face(bot_face_bit); }

  constexpr void front_face(const bool value) {
    _set_face(front_face_bit, value);
  }
  constexpr void back_face(const bool value) {
    _set_face(back_face_bit, value);
  }
  constexpr void left_face(const bool value) {
    _set_face(left_face_bit, value);
  }
  constexpr void right_face(const bool value) {
    _set_face(right_face_bit, value);
  }
  constexpr void top_face(const bool value) { _set_face(top_face_bit, value); }
  constexpr void bot_face(const bool value) { _set_face(bot_face_bit, value); }

  void generate(const block::Server &block_server,
                std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
                const glm::vec3 &position) const;
  physics::AABB to_aabb(const glm::vec3 &position) const;

  block::Type type;

private:
  static constexpr uint8_t front_face_bit = 1 << 0;
  static constexpr uint8_t back_face_bit = 1 << 1;
  static constexpr uint8_t left_face_bit = 1 << 2;
  static constexpr uint8_t right_face_bit = 1 << 3;
  static constexpr uint8_t top_face_bit = 1 << 4;
  static constexpr uint8_t bot_face_bit = 1 << 5;

  static void
  _create_cube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
               const glm::vec3 &position,
               const block::Server::TextureCoordinates &tex_coords,
               const bool front_face = true, const bool back_face = true,
               const bool left_face = true, const bool right_face = true,
               const bool top_face = true, const bool bot_face = true);

  constexpr bool _get_face(const uint8_t bit) const {
    return (m_faces & bit) == bit;
  }
  constexpr void _set_face(const uint8_t bit, const bool value) {
    m_faces = m_faces & (~bit * !value + ~0 * value) | (bit * value);
  }

  uint8_t m_faces;
};

class BlockArray {
public:
  void fill(const block::Type value = block::Type::GRASS);
  void half_fill(const block::Type value = block::Type::GRASS);
  void clear();

  inline block::Type get(const size_t x, const size_t y, const size_t z) const {
    return m_array[_index(x, y, z)].type;
  }

  inline Block &get_block(const size_t x, const size_t y, const size_t z) {
    return m_array[_index(x, y, z)];
  }

  inline const Block &get_block(const size_t x, const size_t y,
                                const size_t z) const {
    return m_array[_index(x, y, z)];
  }

  inline void set(const size_t x, const size_t y, const size_t z,
                  const block::Type value) {
    m_array[_index(x, y, z)].type = value;
  }

  std::array<uint8_t, block_width * block_depth * block_height>
  to_stored_blocks() const;
  void from_stored_blocks(
      const std::array<uint8_t, block_width * block_depth * block_height>
          &stored_blocks);

private:
  static inline size_t _index(const size_t x, const size_t y, const size_t z) {
    return x + z * block_width + y * (block_width * block_depth);
  }

  std::array<Block, block_width * block_depth * block_height> m_array;
};
} // namespace chunk
