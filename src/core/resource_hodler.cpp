#include "resource_hodler.hpp"
#include "../chunk/mesh.hpp"
#include "text/text.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace core {

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

void ResourceHodler::_load_all_resources(const vulkan::Context &context,
                                         const Settings &settings) {
  _load_all_textures(context, settings);
  _load_all_fonts();
  _load_all_shaders(context, settings);
}

} // namespace core