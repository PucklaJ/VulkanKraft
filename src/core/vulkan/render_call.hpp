#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class Context;

class RenderCall {
public:
  inline const uint32_t &get_swap_chain_image_index() const {
    return m_image_index;
  }

  RenderCall(Context *context, const vk::CommandBuffer &graphics_buffer,
             const vk::Framebuffer &framebuffer,
             const uint32_t swap_chain_image_index,
             const vk::Semaphore &image_available_semaphore,
             const vk::Semaphore &render_finished_semaphore,
             const vk::Fence &in_flight_fence);
  ~RenderCall();

  void render_vertices(const uint32_t num_vertices,
                       const uint32_t first_vertex = 0) const noexcept;
  void render_indices(const uint32_t num_indices,
                      const uint32_t first_index = 0) const noexcept;

  void bind_graphics_pipeline(const vk::Pipeline &pipeline) const noexcept;
  void bind_descriptor_set(const vk::DescriptorSet &set,
                           const vk::PipelineLayout &layout,
                           const uint32_t set_index) const noexcept;
  void bind_buffer(const vk::Buffer &buffer, vk::BufferUsageFlags usage) const;

private:
  const vk::CommandBuffer &m_graphics_buffer;
  const uint32_t m_image_index;
  const vk::Semaphore &m_image_available_semaphore;
  const vk::Semaphore &m_render_finished_semaphore;
  const vk::Fence &m_in_flight_fence;

  Context *m_context;
};
} // namespace vulkan
} // namespace core