#include "render_call.hpp"
#include "../exception.hpp"

namespace core {
namespace vulkan {
void RenderCall::render_vertices(const uint32_t num_vertices,
                                 const uint32_t first_vertex) const noexcept {
  m_graphics_buffer.draw(num_vertices, 1, first_vertex, 0);
}

void RenderCall::bind_graphics_pipeline(
    const vk::Pipeline &pipeline) const noexcept {
  m_graphics_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}

void RenderCall::bind_descriptor_set(
    const vk::DescriptorSet &set,
    const vk::PipelineLayout &layout) const noexcept {
  m_graphics_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout,
                                       0, set, nullptr);
}

void RenderCall::bind_buffer(const vk::Buffer &buffer,
                             vk::BufferUsageFlags usage) const {
  if (usage & vk::BufferUsageFlagBits::eIndexBuffer) {
    m_graphics_buffer.bindIndexBuffer(buffer, 0, vk::IndexType::eUint32);
  } else if (usage & vk::BufferUsageFlagBits::eVertexBuffer) {
    m_graphics_buffer.bindVertexBuffers(0, buffer,
                                        static_cast<vk::DeviceSize>(0));
  } else {
    throw VulkanKraftException(
        "invalid buffer usage for core::vulkan::RenderCall::bind_buffer");
  }
}

RenderCall::RenderCall(const vk::CommandBuffer &graphics_buffer,
                       const size_t swap_chain_image_index)
    : m_graphics_buffer(graphics_buffer),
      m_image_index(swap_chain_image_index) {}
} // namespace vulkan
} // namespace core
