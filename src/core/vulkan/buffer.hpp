#pragma once
#include "context.hpp"
#include "render_call.hpp"

namespace core {
namespace vulkan {
class Buffer {
public:
  Buffer(const Context &context, vk::BufferUsageFlags usage,
         const size_t buffer_size, const void *data = nullptr);
  ~Buffer();

  void set_data(const void *data, const size_t data_size,
                const size_t offset = 0);
  void bind(const RenderCall &render_call) const;
  inline const vk::Buffer &get_handle() const { return m_handle; }

private:
  void _create(const vk::DeviceSize buffer_size);
  void _destroy();

  vk::Buffer m_handle;
  vk::DeviceMemory m_memory;
  const vk::BufferUsageFlags m_usage;
  vk::DeviceSize m_buffer_size;

  const Context &m_context;
};
} // namespace vulkan
} // namespace core