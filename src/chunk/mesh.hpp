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
