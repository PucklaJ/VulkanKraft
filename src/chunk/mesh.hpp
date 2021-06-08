#pragma once
#include "../core/resource_hodler.hpp"
#include "../core/shader.hpp"
#include "../core/vulkan/buffer.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace chunk {

class Chunk;

constexpr size_t block_width = 32;
constexpr size_t block_depth = 32;
constexpr size_t block_height = 256;

class BlockArray {
public:
  using BlockType = bool;

  void fill(const BlockType value = true);
  void clear();

  inline BlockType get(const size_t x, const size_t y, const size_t z) const {
    return m_array[x][z][y];
  }

  inline void set(const size_t x, const size_t y, const size_t z,
                  const BlockType value) {
    m_array[x][z][y] = value;
  }

protected:
  std::array<std::array<std::array<BlockType, block_height>, block_depth>,
             block_width>
      m_array;
};

class Mesh {
public:
  class Vertex {
  public:
    Vertex(float x, float y, float z, float u, float v);

    glm::vec3 position;
    glm::vec2 uv;
  };
  struct GlobalUniform {
    glm::mat4 proj_view;
  };

  static ::core::Shader build_shader(const ::core::vulkan::Context &context,
                                     const ::core::Settings &settings,
                                     ::core::ResourceHodler &resource_hodler);

  Mesh(const ::core::vulkan::Context &context);

  void render(const ::core::vulkan::RenderCall &render_call);

  void generate(const Chunk *chunk, const glm::vec2 &pos);

private:
  static void
  _create_cube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
               const glm::vec3 &position, const bool front_face = true,
               const bool back_face = true, const bool left_face = true,
               const bool right_face = true, const bool top_face = true,
               const bool bot_face = true);

  static inline void _check_faces_of_block(const BlockArray *blocks,
                                           const size_t x, const size_t y,
                                           const size_t z, bool &front_face,
                                           bool &back_face, bool &right_face,
                                           bool &left_face, bool &top_face,
                                           bool &bot_face) {
    left_face = x == 0 || !blocks->get(x - 1, y, z);
    right_face = x == block_width - 1 || !blocks->get(x + 1, y, z);

    front_face = z == block_depth - 1 || !blocks->get(x, y, z + 1);
    back_face = z == 0 || !blocks->get(x, y, z - 1);

    top_face = y == block_height - 1 || !blocks->get(x, y + 1, z);
    bot_face = y == 0 || !blocks->get(x, y - 1, z);
  }

  std::unique_ptr<::core::vulkan::Buffer> m_vertex_buffer;
  std::unique_ptr<::core::vulkan::Buffer> m_index_buffer;
  uint32_t m_num_indices;
};
} // namespace chunk
