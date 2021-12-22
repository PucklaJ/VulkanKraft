#pragma once
#include "settings.hpp"
#include "shader.hpp"
#include "text/font.hpp"
#include "texture.hpp"

#include <filesystem>
#include <map>

namespace core {
// Loads all resources and hodles them until the end of time (aka the end of the
// program)
class ResourceHodler {
public:
  // Names of the differnet textures used to store in the map
  static constexpr char chunk_mesh_texture_name[] = "chunk/mesh";
  static constexpr char crosshair_texture_name[] = "crosshair";
  static constexpr char inventory_hotbar_texture_name[] = "inventory_hotbar";
  static constexpr char dirt_block_item_texture_name[] = "dirt_block_item";
  static constexpr char grass_block_item_texture_name[] = "grass_block_item";

  // Names of the differnet fonts used to store in the map
  static constexpr char debug_font_name[] = "debug";

  // Names of the differnet shaders used to store in the map
  static constexpr char chunk_mesh_shader_name[] = "chunk/mesh";
  static constexpr char text_shader_name[] = "text";
  static constexpr char texture_2d_shader_name[] = "texture_2d";
  static constexpr char line_3d_shader_name[] = "line_3d";

  // load_all ...... Wether to load all of the resources
  ResourceHodler(const vulkan::Context &context, const Settings &settings,
                 const bool load_all = true);

  /// Loads a texture with the given data and stores it as name
  // data .................. A texture in PNG format
  // size .................. Size of data in bytes
  // builder_callback ..... Gets called before the texture is built to adjust it
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
  // Loads a font and stores it as name
  // data ..... TTF or OTF format data
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
  void build_line_3d_shader(const vulkan::Context &context,
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
  // Loads all default resources
  void _load_all_resources(const vulkan::Context &context,
                           const Settings &settings);
  void _load_all_textures(const vulkan::Context &context,
                          const Settings &settings);
  void _load_all_fonts();
  void _load_all_shaders(const vulkan::Context &context,
                         const Settings &settings);

  // Stores all shaders that have been loaded
  std::map<std::string, Shader> m_hodled_shaders;
  // Stores all textures that have been loaded
  std::map<std::string, Texture> m_hodled_textures;
  // Store all fonts that have been loaded
  std::map<std::string, text::Font> m_hodled_fonts;

  const vulkan::Context &m_context;
};
} // namespace core