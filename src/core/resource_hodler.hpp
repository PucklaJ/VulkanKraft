#pragma once
#include "settings.hpp"
#include "shader.hpp"
#include "text/font.hpp"
#include "texture.hpp"

#include <filesystem>
#include <map>

namespace core {
class ResourceHodler {
public:
  static constexpr char chunk_mesh_texture_name[] = "chunk/mesh";
  static constexpr char debug_font_name[] = "debug";
  static constexpr char chunk_mesh_shader_name[] = "chunk/mesh";
  static constexpr char text_shader_name[] = "text";
  static constexpr char texture_2d_shader_name[] = "texture_2d";

  ResourceHodler(const vulkan::Context &context, const Settings &settings,
                 const bool load_all = true);

  void load_texture(
      const vulkan::Context &context, std::string name, const uint8_t *data,
      const size_t size,
      std::function<void(Texture::Builder &)> builder_callback = nullptr);
  template <size_t S>
  inline void load_texture(
      const vulkan::Context &context, std::string name,
      const std::array<uint8_t, S> &data,
      std::function<void(Texture::Builder &)> builder_callback = nullptr) {
    load_texture(context, name, data.data(), S, builder_callback);
  }
  void load_font(std::string name, const uint8_t *data);
  template <size_t S>
  inline void load_font(std::string name, const std::array<uint8_t, S> &data) {
    load_font(std::move(name), data.data());
  }

  void build_chunk_mesh_shader(const vulkan::Context &context,
                               const Settings &settings);
  void build_text_shader(const vulkan::Context &context,
                         const Settings &settings);
  void build_texture_2d_shader(const vulkan::Context &context,
                               const Settings &settings);

  inline Texture &get_texture(std::string name) {
    return m_hodled_textures.at(name);
  }
  inline text::Font &get_font(std::string name) {
    return m_hodled_fonts.at(name);
  }
  inline Shader &get_shader(std::string name) {
    return m_hodled_shaders.at(name);
  }

private:
  void _load_all_resources(const vulkan::Context &context,
                           const Settings &settings);

  std::map<std::string, Texture> m_hodled_textures;
  std::map<std::string, text::Font> m_hodled_fonts;
  std::map<std::string, Shader> m_hodled_shaders;

  const vulkan::Context &m_context;
};
} // namespace core