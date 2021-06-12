#pragma once
#include "../core/resource_hodler.hpp"
#include "../core/shader.hpp"
#include "../core/vulkan/buffer.hpp"
#include "block.hpp"
#include <atomic>
#include <glm/glm.hpp>
#include <memory>
#include <thread>

namespace chunk {

class Chunk;

class Mesh {
public:
  static ::core::Shader build_shader(const ::core::vulkan::Context &context,
                                     const ::core::Settings &settings,
                                     ::core::ResourceHodler &resource_hodler);

  Mesh(const ::core::vulkan::Context &context);
  ~Mesh();

  void render(const ::core::vulkan::RenderCall &render_call);

  void generate(Chunk *chunk, const glm::vec2 &pos,
                const bool multi_thread = true);

private:
  std::unique_ptr<::core::vulkan::Buffer> m_vertex_buffer;
  std::unique_ptr<::core::vulkan::Buffer> m_index_buffer;
  uint32_t m_num_indices;

  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::atomic<bool> m_vertices_ready;
  bool m_generating;
  std::unique_ptr<std::thread> m_generate_thread;
};
} // namespace chunk
