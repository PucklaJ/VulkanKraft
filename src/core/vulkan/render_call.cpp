#include "render_call.hpp"
#include "../exception.hpp"
#include "context.hpp"

namespace core {
namespace vulkan {
RenderCall::RenderCall(Context *context,
                       const vk::CommandBuffer &graphics_buffer,
                       const vk::Framebuffer &framebuffer,
                       const uint32_t swap_chain_image_index,
                       const vk::Semaphore &image_available_semaphore,
                       const vk::Semaphore &render_finished_semaphore,
                       const vk::Fence &in_flight_fence)
    : m_graphics_buffer(graphics_buffer), m_image_index(swap_chain_image_index),
      m_image_available_semaphore(image_available_semaphore),
      m_render_finished_semaphore(render_finished_semaphore),
      m_in_flight_fence(in_flight_fence), m_context(context) {
  m_graphics_buffer.begin(vk::CommandBufferBeginInfo());

  vk::RenderPassBeginInfo rbi;
  rbi.renderPass = m_context->get_swap_chain_render_pass();
  rbi.framebuffer = framebuffer;
  rbi.renderArea.extent = m_context->m_swap_chain->get_extent();

  std::array<vk::ClearValue, 2> clear_values;
  clear_values[0].color = std::array{
      54.0f / 255.0f,
      197.0f / 255.0f,
      244.0f / 255.0f,
      1.0f,
  };
  clear_values[1].depthStencil.depth = 1.0f;
  rbi.clearValueCount = static_cast<uint32_t>(clear_values.size());
  rbi.pClearValues = clear_values.data();

  m_graphics_buffer.beginRenderPass(rbi, vk::SubpassContents::eInline);

  vk::Viewport view;
  view.width = static_cast<float>(m_context->m_swap_chain->get_extent().width);
  view.height =
      static_cast<float>(m_context->m_swap_chain->get_extent().height);
  view.minDepth = 0.0f;
  view.maxDepth = 1.0f;
  view.x = 0.0f;
  view.y = 0.0f;

  vk::Rect2D scissor;
  scissor.extent = m_context->m_swap_chain->get_extent();

  m_graphics_buffer.setViewport(0, view);
  m_graphics_buffer.setScissor(0, scissor);
}

RenderCall::~RenderCall() {
  m_graphics_buffer.endRenderPass();
  m_graphics_buffer.end();

  vk::SubmitInfo si;

  const auto wait_sems = std::array{m_image_available_semaphore};
  const auto wait_stages = std::array<vk::PipelineStageFlags, wait_sems.size()>{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  si.waitSemaphoreCount = static_cast<uint32_t>(wait_sems.size());
  si.pWaitSemaphores = wait_sems.data();
  si.pWaitDstStageMask = wait_stages.data();
  si.commandBufferCount = 1;
  si.pCommandBuffers = &m_graphics_buffer;

  const auto sig_sems = std::array{m_render_finished_semaphore};
  si.signalSemaphoreCount = static_cast<uint32_t>(sig_sems.size());
  si.pSignalSemaphores = sig_sems.data();

  m_context->m_device.resetFences(m_in_flight_fence);

  m_context->m_graphics_queue.submit(si, m_in_flight_fence);

  vk::PresentInfoKHR pi;
  pi.waitSemaphoreCount = static_cast<uint32_t>(sig_sems.size());
  pi.pWaitSemaphores = sig_sems.data();

  const auto swap_chains = std::array{m_context->m_swap_chain->get_handle()};
  pi.swapchainCount = static_cast<uint32_t>(swap_chains.size());
  pi.pSwapchains = swap_chains.data();
  pi.pImageIndices = &m_image_index;

  try {
    if (const auto r = m_context->m_present_queue.presentKHR(pi);
        r == vk::Result::eErrorOutOfDateKHR ||
        r == vk::Result::eSuboptimalKHR || m_context->m_framebuffer_resized) {
      m_context->_handle_framebuffer_resize();
      m_context->m_framebuffer_resized = false;
    }
  } catch (const vk::OutOfDateKHRError &e) {
    m_context->_handle_framebuffer_resize();
    m_context->m_framebuffer_resized = false;
  }

  m_context->m_current_frame =
      (m_context->m_current_frame + 1) % Context::_max_images_in_flight;
}

void RenderCall::render_vertices(const uint32_t num_vertices,
                                 const uint32_t first_vertex) const noexcept {
  m_graphics_buffer.draw(num_vertices, 1, first_vertex, 0);
}

void RenderCall::render_indices(const uint32_t num_indices,
                                const uint32_t first_index) const noexcept {
  m_graphics_buffer.drawIndexed(num_indices, 1, first_index, 0, 0);
}

void RenderCall::bind_graphics_pipeline(
    const vk::Pipeline &pipeline) const noexcept {
  m_graphics_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}

void RenderCall::bind_descriptor_set(const vk::DescriptorSet &set,
                                     const vk::PipelineLayout &layout,
                                     const uint32_t set_index) const noexcept {
  m_graphics_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout,
                                       set_index, set, nullptr);
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

} // namespace vulkan
} // namespace core
