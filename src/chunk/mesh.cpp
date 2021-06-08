#include "mesh.hpp"
#include "chunk.hpp"
#include <array>
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace chunk {

void BlockArray::fill(const BlockType value) {
  for (auto &x : m_array) {
    for (auto &z : x) {
      std::fill(z.begin(), z.end(), value);
    }
  }
}

void BlockArray::clear() { fill(false); }

Mesh::Vertex::Vertex(float x, float y, float z, float u, float v)
    : position(x, y, z), uv(u, v) {}

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

Mesh::Mesh(const ::core::vulkan::Context &context) {
  m_vertex_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eVertexBuffer, 1);
  m_index_buffer = std::make_unique<::core::vulkan::Buffer>(
      context, vk::BufferUsageFlagBits::eIndexBuffer, 1);
}

void Mesh::render(const ::core::vulkan::RenderCall &render_call) {
  m_vertex_buffer->bind(render_call);
  m_index_buffer->bind(render_call);
  render_call.render_indices(m_num_indices);
}

void Mesh::generate(const Chunk *chunk, const glm::vec2 &pos) {
  std::vector<Vertex> vertices;
  vertices.reserve(block_depth * block_width * block_height * 8);
  std::vector<uint32_t> indices;
  indices.reserve(block_depth * block_width * block_height * 36);

  for (size_t x = 0; x < block_width; x++) {
    for (size_t z = 0; z < block_depth; z++) {
      for (size_t y = 0; y < block_height; y++) {
        if (chunk->get(x, y, z)) {
          bool front_face;
          bool back_face;
          bool right_face;
          bool left_face;
          bool top_face;
          bool bot_face;

          _check_faces_of_block(chunk, x, y, z, front_face, back_face,
                                right_face, left_face, top_face, bot_face);

          _create_cube(vertices, indices,
                       glm::vec3(static_cast<float>(x) + pos.x + 0.5f,
                                 static_cast<float>(y) + pos.y + 0.5f,
                                 static_cast<float>(z) + 0.0f + 0.5f),
                       front_face, back_face, left_face, right_face, top_face,
                       bot_face);
        }
      }
    }
  }
  m_vertex_buffer->set_data(vertices.data(), sizeof(Vertex) * vertices.size());
  m_index_buffer->set_data(indices.data(), sizeof(uint32_t) * indices.size());
  m_num_indices = indices.size();
}

void Mesh::_create_cube(std::vector<Mesh::Vertex> &vertices,
                        std::vector<uint32_t> &indices, const glm::vec3 &p,
                        const bool front_face, const bool back_face,
                        const bool left_face, const bool right_face,
                        const bool top_face, const bool bot_face) {
  if (!(front_face || back_face || left_face || right_face || top_face ||
        bot_face))
    return;

  vertices.reserve(vertices.size() + 8);

  const auto i{vertices.size()};
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f, 1.0f);  // 0
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f, 1.0f);   // 1
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 0.0f);    // 2
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 0.0f);   // 3
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f, 1.0f); // 4
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f, 1.0f);  // 5
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, 0.0f, 0.0f);   // 6
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f, 1.0f, 0.0f);  // 7
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 1.0f);   // 8
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 1.0f);    // 9
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, 1.0f, 0.0f); // 10
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f, 0.0f,
                        0.0f);                                             // 11
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f, 1.0f); // 12
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f, 1.0f);  // 13
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f, 0.0f); // 14
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f,
                        0.0f);                                             // 15
  vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f, 1.0f); // 16
  vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 0.0f);   // 17
  vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f,
                        1.0f);                                            // 18
  vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 0.0f); // 19

  if (front_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 0);
  }

  if (back_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 5);
    indices.emplace_back(i + 4);
    indices.emplace_back(i + 7);
    indices.emplace_back(i + 7);
    indices.emplace_back(i + 6);
    indices.emplace_back(i + 5);
  }

  if (right_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 13);
    indices.emplace_back(i + 16);
    indices.emplace_back(i + 10);
    indices.emplace_back(i + 10);
    indices.emplace_back(i + 17);
    indices.emplace_back(i + 13);
  }

  if (left_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 18);
    indices.emplace_back(i + 12);
    indices.emplace_back(i + 19);
    indices.emplace_back(i + 19);
    indices.emplace_back(i + 11);
    indices.emplace_back(i + 18);
  }

  if (top_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 8);
    indices.emplace_back(i + 9);
    indices.emplace_back(i + 10);
    indices.emplace_back(i + 10);
    indices.emplace_back(i + 11);
    indices.emplace_back(i + 8);
  }

  if (bot_face) {
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 15);
    indices.emplace_back(i + 14);
    indices.emplace_back(i + 13);
    indices.emplace_back(i + 13);
    indices.emplace_back(i + 12);
    indices.emplace_back(i + 15);
  }
}
}; // namespace chunk