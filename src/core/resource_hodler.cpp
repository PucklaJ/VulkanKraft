#include "resource_hodler.hpp"
#include "../chunk/mesh.hpp"
#include "text/text.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace core {

namespace shaders {
#include <shaders/chunk_mesh_frag.hpp>
#include <shaders/chunk_mesh_vert.hpp>
#include <shaders/text_frag.hpp>
#include <shaders/text_vert.hpp>
#include <shaders/texture_2d_frag.hpp>
#include <shaders/texture_2d_vert.hpp>
} // namespace shaders

namespace textures {
#include <textures/block.hpp>
}

namespace fonts {
#include <fonts/MisterPixelRegular.hpp>
}

ResourceHodler::ResourceHodler(const core::vulkan::Context &context,
                               const Settings &settings, const bool load_all)
    : m_context(context) {
  if (load_all)
    _load_all_resources(context, settings);
}

void ResourceHodler::load_texture(
    const vulkan::Context &context, std::string name, const uint8_t *data,
    const size_t size,
    std::function<void(Texture::Builder &)> builder_callback) {
  int texture_width, texture_height, texture_channels;
  void *texture_data = stbi_load_from_memory(
      reinterpret_cast<stbi_uc const *>(data), size, &texture_width,
      &texture_height, &texture_channels, STBI_rgb_alpha);

  ::core::Texture::Builder builder;
  builder.dimensions(texture_width, texture_height);
  if (builder_callback)
    builder_callback(builder);
  auto texture(builder.build(context, texture_data));

  stbi_image_free(texture_data);

  m_hodled_textures.emplace(std::move(name), std::move(texture));
}

void ResourceHodler::load_font(std::string name, const uint8_t *data) {
  m_hodled_fonts.emplace(std::move(name), text::Font(data));
}

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
  const glm::mat4 proj_model(1.0f);
  constexpr uint32_t max_textures = 10;

  auto shader(Shader::Builder()
                  .vertex(shaders::texture_2d_vert_spv)
                  .fragment(shaders::texture_2d_frag_spv)
                  .uniform_buffer(vk::ShaderStageFlagBits::eVertex, proj_model)
                  .dynamic_texture(max_textures)
                  .alpha_blending()
                  .build(context, settings));

  m_hodled_shaders.emplace(texture_2d_shader_name, std::move(shader));
}

void ResourceHodler::_load_all_resources(const vulkan::Context &context,
                                         const Settings &settings) {
  // Textures
  load_texture(context, chunk_mesh_texture_name, textures::block_png,
               [](auto &b) { b.filter(vk::Filter::eNearest).mip_maps(); });

  // Fonts
  load_font(debug_font_name, fonts::MisterPixelRegular_otf);

  // Shaders
  build_chunk_mesh_shader(context, settings);
  build_text_shader(context, settings);
  build_texture_2d_shader(context, settings);
}

} // namespace core