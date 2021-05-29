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
  std::unique_ptr<::core::vulkan::Buffer> m_vertex_buffer;
  std::unique_ptr<::core::vulkan::Buffer> m_index_buffer;
  uint32_t num_indices;
};
} // namespace chunk
