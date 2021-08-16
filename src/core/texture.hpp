#pragma once
#include "vulkan/context.hpp"
#include <set>

namespace core {
// A class representing image data on the GPU ready to be sampled in a shader
class Texture {
public:
  friend class Shader;

  // A class used to create a Texture
  class Builder {
  public:
    friend class Texture;

    Builder();

    // Configure the dimensions of the Texture in pixels
    inline Builder &dimensions(const uint32_t width, const uint32_t height) {
      m_width = width;
      m_height = height;
      return *this;
    }

    // Configure the filtering mode of the Texture
    inline Builder &filter(const vk::Filter filter) {
      m_filter = filter;
      return *this;
    }

    // Configure the address mode of the Texture
    inline Builder &address_mode(const vk::SamplerAddressMode address_mode) {
      m_address_mode = address_mode;
      return *this;
    }

    // Configure the amount of anisotropic filtering
    inline Builder &anisotropy(const float max_anisotropy) {
      m_max_anisotropy = max_anisotropy;
      return *this;
    }

    // Configure the border color of the Texture
    inline Builder &border_color(const vk::BorderColor border_color) {
      m_border_color = border_color;
      return *this;
    }

    // Enable mip maps for the Texture
    inline Builder &mip_maps() {
      m_mip_levels = 2;
      return *this;
    }

    // Configure what mip map mode should be used for the Texture
    inline Builder &mip_map_mode(const vk::SamplerMipmapMode mip_mode) {
      m_mip_mode = mip_mode;
      return *this;
    }

    // Configure the format of the Texture
    inline Builder &format(const vk::Format format) {
      m_format = format;
      return *this;
    }

    // Create the Texture using texture data
    Texture build(const vulkan::Context &context, const void *data);

  private:
    uint32_t m_width;
    uint32_t m_height;
    vk::Filter m_filter;
    vk::SamplerAddressMode m_address_mode;
    float m_max_anisotropy;
    vk::BorderColor m_border_color;
    uint32_t m_mip_levels;
    vk::SamplerMipmapMode m_mip_mode;
    vk::Format m_format;
  };

  Texture(Texture &&rhs);
  Texture &operator=(Texture &&rhs);
  ~Texture();

  // Create a descriptor image info to be used to update the shader
  inline vk::DescriptorImageInfo create_descriptor_image_info() const {
    vk::DescriptorImageInfo ii;
    ii.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    ii.imageView = m_image_view;
    ii.sampler = m_sampler;
    return ii;
  }

  // Move the dynamic sets out of the Texture
  inline std::vector<vk::DescriptorSet> extract_dynamic_sets() {
    return std::move(m_dynamic_sets);
  }

  // Set the dynamic sets of the Texture
  inline void set_dynamic_sets(std::vector<vk::DescriptorSet> sets) {
    m_dynamic_sets = std::move(sets);
    m_dynamic_writes_to_perform.clear();
    for (uint32_t i = 0; i < m_dynamic_sets.size(); i++) {
      m_dynamic_writes_to_perform.emplace(i);
    }
  }

  // Returns the width of the texture in pixels
  inline uint32_t get_width() const { return m_width; }
  // Returns the height of the texture in pixels
  inline uint32_t get_height() const { return m_height; }

  // Create the Texture anew
  void rebuild(const Builder &builder, const void *data);

private:
  // Returns the image buffer size (in bytes) necessary for an image with the
  // given dimensions and format
  static uint32_t _get_image_size(const uint32_t width, const uint32_t height,
                                  const vk::Format format);

  // Make the constructor private so that only the Builder can create Textures
  Texture(const vulkan::Context &context, const Builder &builder,
          const void *data);

  void _create_image(const Builder &builder, const void *data);
  void _create_image_view(const Builder &builder);
  void _create_sampler(const Builder &builder);
  void _generate_mip_maps(const Builder &builder);
  void _create_descriptor_sets(const vk::DescriptorPool &pool,
                               const vk::DescriptorSetLayout &layout,
                               const uint32_t binding_point);
  void _write_dynamic_set(const size_t index);

  void _destroy();

  vk::Image m_image;
  vk::ImageView m_image_view;
  vk::DeviceMemory m_memory;
  vk::Sampler m_sampler;
  // Descriptor sets for the shader
  std::vector<vk::DescriptorSet> m_dynamic_sets;
  // What binding point the shader should use
  uint32_t m_dynamic_binding_point;
  // Which descriptor sets should be updated (every element represents a swap
  // chain image index)
  std::set<uint32_t> m_dynamic_writes_to_perform;

  uint32_t m_width;
  uint32_t m_height;

  const vulkan::Context &m_context;
};
} // namespace core
