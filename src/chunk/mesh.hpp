#pragma once
#include "../core/resource_hodler.hpp"
#include "../core/shader.hpp"
#include "../core/vulkan/buffer.hpp"
#include "block.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace chunk {

class Chunk;

class Mesh {
public:
  static ::core::Shader build_shader(const ::core::vulkan::Context &context,
                                     const ::core::Settings &settings,
                                     ::core::ResourceHodler &resource_hodler);

  Mesh(const ::core::vulkan::Context &context);

  void render(const ::core::vulkan::RenderCall &render_call);

  void generate(Chunk *chunk, const glm::vec2 &pos);

private:
  std::unique_ptr<::core::vulkan::Buffer> m_vertex_buffer;
  std::unique_ptr<::core::vulkan::Buffer> m_index_buffer;
  uint32_t m_num_indices;
};
} // namespace chunk
