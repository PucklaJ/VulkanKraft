#pragma once
#include "vulkan/context.hpp"
#include <set>

namespace core {
class Texture {
public:
  friend class Shader;
  class Builder {
  public:
    friend class Texture;

    Builder();

    inline Builder &dimensions(const uint32_t width, const uint32_t height) {
      m_width = width;
      m_height = height;
      return *this;
    }

    inline Builder &filter(const vk::Filter filter) {
      m_filter = filter;
      return *this;
    }

    inline Builder &address_mode(const vk::SamplerAddressMode address_mode) {
      m_address_mode = address_mode;
      return *this;
    }

    inline Builder &anisotropy(const float max_anisotropy) {
      m_max_anisotropy = max_anisotropy;
      return *this;
    }

    inline Builder &border_color(const vk::BorderColor border_color) {
      m_border_color = border_color;
      return *this;
    }

    inline Builder &mip_maps() {
      m_mip_levels = 2;
      return *this;
    }

    inline Builder &mip_map_mode(const vk::SamplerMipmapMode mip_mode) {
      m_mip_mode = mip_mode;
      return *this;
    }

    inline Builder &format(const vk::Format format) {
      m_format = format;
      return *this;
    }

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

  inline vk::DescriptorImageInfo create_descriptor_image_info() const {
    vk::DescriptorImageInfo ii;
    ii.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    ii.imageView = m_image_view;
    ii.sampler = m_sampler;
    return ii;
  }

  inline std::vector<vk::DescriptorSet> extract_dynamic_sets() {
    return std::move(m_dynamic_sets);
  }
  inline void set_dynamic_sets(std::vector<vk::DescriptorSet> sets) {
    m_dynamic_sets = std::move(sets);
    m_dynamic_writes_to_perform.clear();
    for (uint32_t i = 0; i < m_dynamic_sets.size(); i++) {
      m_dynamic_writes_to_perform.emplace(i);
    }
  }

  void rebuild(const Builder &builder, const void *data);

private:
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
  std::vector<vk::DescriptorSet> m_dynamic_sets;
  uint32_t m_dynamic_binding_point;
  std::set<uint32_t> m_dynamic_writes_to_perform;

  const vulkan::Context &m_context;
};
} // namespace core
