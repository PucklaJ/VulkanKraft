#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class RenderCall {
public:
  friend class Context;

  inline const size_t &get_swap_chain_image_index() const {
    return m_image_index;
  }

  void render_vertices(const uint32_t num_vertices,
                       const uint32_t first_vertex = 0) const noexcept;

  void bind_graphics_pipeline(const vk::Pipeline &pipeline) const noexcept;
  void bind_descriptor_set(const vk::DescriptorSet &set,
                           const vk::PipelineLayout &layout) const noexcept;
  void bind_buffer(const vk::Buffer &buffer, vk::BufferUsageFlags usage) const;

private:
  RenderCall(const vk::CommandBuffer &graphics_buffer,
             const size_t swap_chain_image_index);

  const vk::CommandBuffer &m_graphics_buffer;
  const size_t m_image_index;
};
} // namespace vulkan
} // namespace core