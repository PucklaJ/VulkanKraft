#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>

namespace chunk {

constexpr int block_width = 16;
constexpr int block_depth = 16;
constexpr int block_height = 128;

class Vertex {
public:
  Vertex(float x, float y, float z, float u, float v);

  glm::vec3 position;
  glm::vec2 uv;
};
struct GlobalUniform {
  glm::mat4 proj_view;
};

enum BlockType {
  AIR,
  GRASS,
};

class Block {
public:
  static constexpr size_t default_face_count =
      (block_width * block_depth) * 2 + (block_height * block_width) * 2 +
      (block_height * block_depth) * 2;
  static constexpr size_t vertices_per_face = 4;
  static constexpr size_t indices_per_face = 6;

  Block();

  constexpr inline bool front_face() const { return m_faces[front_face_index]; }
  constexpr inline bool back_face() const { return m_faces[back_face_index]; }
  constexpr inline bool left_face() const { return m_faces[left_face_index]; }
  constexpr inline bool right_face() const { return m_faces[right_face_index]; }
  constexpr inline bool top_face() const { return m_faces[top_face_index]; }
  constexpr inline bool bot_face() const { return m_faces[bot_face_index]; }

  constexpr inline bool &front_face() { return m_faces[front_face_index]; }
  constexpr inline bool &back_face() { return m_faces[back_face_index]; }
  constexpr inline bool &left_face() { return m_faces[left_face_index]; }
  constexpr inline bool &right_face() { return m_faces[right_face_index]; }
  constexpr inline bool &top_face() { return m_faces[top_face_index]; }
  constexpr inline bool &bot_face() { return m_faces[bot_face_index]; }

  void generate(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
                const glm::vec3 &position) const;

  BlockType type;

private:
  static constexpr size_t front_face_index = 0;
  static constexpr size_t back_face_index = 1;
  static constexpr size_t left_face_index = 2;
  static constexpr size_t right_face_index = 3;
  static constexpr size_t top_face_index = 4;
  static constexpr size_t bot_face_index = 5;

  static void
  _create_cube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
               const glm::vec3 &position, const bool front_face = true,
               const bool back_face = true, const bool left_face = true,
               const bool right_face = true, const bool top_face = true,
               const bool bot_face = true);

  std::array<bool, 6> m_faces;
};

class BlockArray {
public:
  void fill(const BlockType value = BlockType::GRASS);
  void clear();

  inline BlockType get(const size_t x, const size_t y, const size_t z) const {
    return m_array[x][z][y].type;
  }

  inline Block &get_block(const size_t x, const size_t y, const size_t z) {
    return m_array[x][z][y];
  }

  inline const Block &get_block(const size_t x, const size_t y,
                                const size_t z) const {
    return m_array[x][z][y];
  }

  inline void set(const size_t x, const size_t y, const size_t z,
                  const BlockType value) {
    m_array[x][z][y].type = value;
  }

protected:
  std::array<std::array<std::array<Block, block_height>, block_depth>,
             block_width>
      m_array;
};
} // namespace chunk
