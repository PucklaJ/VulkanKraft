#include "mesh.hpp"
#include "../core/log.hpp"
#include "chunk.hpp"
#include <array>
#include <cstdlib>
#ifndef NDEBUG
#include <chrono>
#include <sstream>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace chunk {

::core::Shader Mesh::build_shader(const ::core::vulkan::Context &context,
                                  const ::core::Settings &settings,
                                  ::core::ResourceHodler &resource_hodler) {
  GlobalUniform global;
  global.proj_view = glm::mat4(1.0f);

  int texture_width, texture_height, texture_channels;
  constexpr char texture_file_name[] = "textures/block.png";
  void *texture_data =
      stbi_load(texture_file_name, &texture_width, &texture_height,
                &texture_channels, STBI_rgb_alpha);
  auto texture = ::core::Texture::Builder()
                     .dimensions(texture_width, texture_height)
                     .filter(vk::Filter::eNearest)
                     .mip_maps()
                     .build(context, texture_data);
  stbi_image_free(texture_data);

  auto shader(::core::Shader::Builder()
                  .vertex_attribute<glm::vec3>()
                  .vertex_attribute<glm::vec2>()
                  .vertex("shaders_spv/chunk_mesh.vert.spv")
                  .fragment("shaders_spv/chunk_mesh.frag.spv")
                  .uniform_buffer(vk::ShaderStageFlagBits::eVertex, global)
                  .texture()
                  .build(context, settings));
  shader.set_texture(texture);

  resource_hodler.hodl_texture("chunk_mesh_texture", std::move(texture));

  return shader;
}

Mesh::Mesh(const ::core::vulkan::Context &context)
    : m_num_indices(-1), m_generating(false) {
  m_vertex_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eVertexBuffer, 1);
  m_index_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eIndexBuffer, 1);
}

Mesh::~Mesh() {
  if (m_generating && !m_vertices_ready && m_generate_thread) {
    m_generate_thread->join();
  }
}

void Mesh::render(const ::core::vulkan::RenderCall &render_call) {
  if (m_generating && m_vertices_ready) {
    if (m_generate_thread) {
      m_generate_thread->join();
      m_generate_thread.reset();
    }

    m_vertex_buffer->set_data(m_vertices.data(),
                              sizeof(Vertex) * m_vertices.size());
    m_index_buffer->set_data(m_indices.data(),
                             sizeof(uint32_t) * m_indices.size());
    m_num_indices = m_indices.size();

    m_vertices.clear();
    m_indices.clear();
    m_generating = false;
  }

  if (!m_generating) {
    m_vertex_buffer->bind(render_call);
    m_index_buffer->bind(render_call);
    render_call.render_indices(m_num_indices);
  }
}

void Mesh::generate(Chunk *chunk, const glm::vec2 &pos,
                    const bool multi_thread) {
  if (m_generating && m_generate_thread) {
    m_generate_thread->join();
    m_generate_thread.reset();
  }

  m_vertices_ready = false;
  m_generating = true;

  m_generate_thread = std::make_unique<std::thread>([&, chunk, pos]() {
#ifndef NDEBUG
    const auto start_time(std::chrono::high_resolution_clock::now());
#endif
    if (m_num_indices != -1) {
      m_vertices.reserve(m_num_indices / Block::indices_per_face *
                         Block::vertices_per_face);
      m_indices.reserve(m_num_indices);
    } else {
      m_vertices.reserve(Block::default_face_count * Block::vertices_per_face);
      m_indices.reserve(Block::default_face_count * Block::indices_per_face);
    }

    for (size_t x = 0; x < block_width; x++) {
      for (size_t z = 0; z < block_depth; z++) {
        for (size_t y = 0; y < block_height; y++) {
          if (auto &block = chunk->get_block(x, y, z);
              block.type != BlockType::AIR) {
            block.generate(m_vertices, m_indices,
                           glm::vec3(static_cast<float>(x) + pos.x + 0.5f,
                                     static_cast<float>(y) + 0.0f + 0.5f,
                                     static_cast<float>(z) + pos.y + 0.5f));
          }
        }
      }
    }

    m_vertices_ready = true;
#ifndef NDEBUG
    const auto end_time(std::chrono::high_resolution_clock::now());
    std::stringstream stream;
    stream << "Mesh Generation Time: "
           << std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                    start_time)
                  .count()
           << " µs";
    ::core::Log::info(stream.str());
#endif
  });

  if (!multi_thread) {
    m_generate_thread->join();
    m_generate_thread.reset();
  }
}
}; // namespace chunk