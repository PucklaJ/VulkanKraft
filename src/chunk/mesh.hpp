#pragma once
#include "../core/shader.hpp"
#include "../core/vulkan/buffer.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace chunk {
class Mesh {
public:
  class Vertex {
  public:
    Vertex(float x, float y, float z);

    glm::vec3 position;
  };
  struct GlobalUniform {
    glm::mat4 proj_view;
  };

  static ::core::Shader build_shader(const ::core::vulkan::Context &context,
                                     const ::core::Settings &settings);

  Mesh(const ::core::vulkan::Context &context);

  void render(const ::core::vulkan::RenderCall &render_call);
  template <size_t width, size_t depth, size_t height>
  void generate(const std::array<std::array<std::array<bool, height>, depth>,
                                 width> &blocks,
                const glm::vec3 &pos) {
    std::vector<Vertex> vertices;
    vertices.reserve(depth * width * height * 8);
    std::vector<uint32_t> indices;
    indices.reserve(depth * width * height * 36);

    for (size_t x = 0; x < width; x++) {
      for (size_t z = 0; z < depth; z++) {
        for (size_t y = 0; y < height; y++) {
          if (blocks[x][z][y]) {
            _create_cube(vertices, indices,
                         glm::vec3(static_cast<float>(x) + pos.x + 0.5f,
                                   static_cast<float>(y) + pos.y + 0.5f,
                                   static_cast<float>(z) + pos.z + 0.5f));
          }
        }
      }
    }
    m_vertex_buffer->set_data(vertices.data(),
                              sizeof(Vertex) * vertices.size());
    m_index_buffer->set_data(indices.data(), sizeof(uint32_t) * indices.size());
    m_num_indices = indices.size();
  }

private:
  static void
  _create_cube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
               const glm::vec3 &position, const bool front_face = true,
               const bool back_face = true, const bool left_face = true,
               const bool right_face = true, const bool top_face = true,
               const bool bot_face = true);

  std::unique_ptr<::core::vulkan::Buffer> m_vertex_buffer;
  std::unique_ptr<::core::vulkan::Buffer> m_index_buffer;
  uint32_t m_num_indices;
};
} // namespace chunk
