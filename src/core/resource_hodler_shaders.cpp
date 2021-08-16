#include "../chunk/mesh.hpp"
#include "resource_hodler.hpp"
#include "text/text.hpp"

namespace shaders {
#include <shaders/chunk_mesh_frag.hpp>
#include <shaders/chunk_mesh_vert.hpp>
#include <shaders/text_frag.hpp>
#include <shaders/text_vert.hpp>
#include <shaders/texture_2d_frag.hpp>
#include <shaders/texture_2d_vert.hpp>
} // namespace shaders

namespace core {
void ResourceHodler::build_chunk_mesh_shader(const vulkan::Context &context,
                                             const Settings &settings) {
  chunk::GlobalUniform global;
  global.proj_view = glm::mat4(1.0f);

  auto shader(::core::Shader::Builder()
                  .vertex_attribute<glm::vec3>()
                  .vertex_attribute<glm::vec2>()
                  .vertex(shaders::chunk_mesh_vert_spv)
                  .fragment(shaders::chunk_mesh_frag_spv)
                  .uniform_buffer(vk::ShaderStageFlagBits::eVertex, global)
                  .texture()
                  .build(context, settings));
  shader.set_texture(get_texture(chunk_mesh_texture_name));

  m_hodled_shaders.emplace(chunk_mesh_shader_name, std::move(shader));
}

void ResourceHodler::build_text_shader(const vulkan::Context &context,
                                       const Settings &settings) {
  text::Text::GlobalUniform global;
  global.proj = glm::mat4(1.0f);

  auto shader(Shader::Builder()
                  .vertex_attribute<glm::vec2>()
                  .vertex_attribute<glm::vec2>()
                  .vertex(shaders::text_vert_spv)
                  .fragment(shaders::text_frag_spv)
                  .uniform_buffer(vk::ShaderStageFlagBits::eVertex, global)
                  .dynamic_texture(text::Text::max_texts)
                  .alpha_blending()
                  .build(context, settings));

  m_hodled_shaders.emplace(text_shader_name, std::move(shader));
}

void ResourceHodler::build_texture_2d_shader(const vulkan::Context &context,
                                             const Settings &settings) {
  const glm::mat4 proj(1.0f);
  constexpr uint32_t max_textures = 10;

  auto shader(Shader::Builder()
                  .vertex(shaders::texture_2d_vert_spv)
                  .fragment(shaders::texture_2d_frag_spv)
                  .uniform_buffer(vk::ShaderStageFlagBits::eVertex, proj)
                  .push_constant<glm::mat4>(vk::ShaderStageFlagBits::eVertex)
                  .dynamic_texture(max_textures)
                  .alpha_blending()
                  .build(context, settings));

  m_hodled_shaders.emplace(texture_2d_shader_name, std::move(shader));
}

void ResourceHodler::_load_all_shaders(const vulkan::Context &context,
                                       const Settings &settings) {
  // Shaders
  build_chunk_mesh_shader(context, settings);
  build_text_shader(context, settings);
  build_texture_2d_shader(context, settings);
}
} // namespace core