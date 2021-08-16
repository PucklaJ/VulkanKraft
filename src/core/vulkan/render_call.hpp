#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class Context;

// This class represents a draw call for one frame. It can only and should only
// be instanciated using Context::render_begin
class RenderCall {
public:
  // Returns the index of the currently used swap chain image
  inline const uint32_t &get_swap_chain_image_index() const {
    return m_image_index;
  }

  RenderCall(Context *context, const vk::CommandBuffer &graphics_buffer,
             const vk::Framebuffer &framebuffer,
             const uint32_t swap_chain_image_index,
             const vk::Semaphore &image_available_semaphore,
             const vk::Semaphore &render_finished_semaphore,
             const vk::Fence &in_flight_fence);
  // Stops recording commands and executes the render call
  ~RenderCall();

  // Render an given amount of vertices
  void render_vertices(const uint32_t num_vertices,
                       const uint32_t first_vertex = 0) const noexcept;
  // Render an given amount of indices
  void render_indices(const uint32_t num_indices,
                      const uint32_t first_index = 0) const noexcept;

  // Bind the given pipeline
  void bind_graphics_pipeline(const vk::Pipeline &pipeline) const noexcept;
  // Bind the given descriptor set
  void bind_descriptor_set(const vk::DescriptorSet &set,
                           const vk::PipelineLayout &layout,
                           const uint32_t set_index) const noexcept;
  // Bind a buffer either as vertex or index or both
  void bind_buffer(const vk::Buffer &buffer, vk::BufferUsageFlags usage) const;

  template <typename T>
  inline void set_push_constant(const vk::PipelineLayout &layout,
                                const vk::ShaderStageFlags stage_flags,
                                const uint32_t offset, const T &data) const {
    m_graphics_buffer.pushConstants(layout, stage_flags, offset, sizeof(data),
                                    &data);
  }

private:
  // The command buffer used to record all commands
  const vk::CommandBuffer &m_graphics_buffer;
  // The current swap chain image index
  const uint32_t m_image_index;
  // The semaphores used to sync rendering and presenting
  const vk::Semaphore &m_image_available_semaphore;
  const vk::Semaphore &m_render_finished_semaphore;
  const vk::Fence &m_in_flight_fence;

  Context *m_context;
};
} // namespace vulkan
} // namespace core