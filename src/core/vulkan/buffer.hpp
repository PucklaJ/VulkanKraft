#pragma once
#include "context.hpp"
#include "render_call.hpp"

namespace core {
namespace vulkan {
// Represents an allocated amount of memory used for a buffer
class Buffer {
public:
  // Initialise the buffer and creates it if data is not nullptr
  Buffer(const Context &context, vk::BufferUsageFlags usage,
         const size_t buffer_size, const void *data = nullptr);
  Buffer(Buffer &&rhs);
  ~Buffer();

  // Update the data of the buffer. Recreate it if the size changed
  void set_data(const void *data, const size_t data_size,
                const size_t offset = 0);
  // Bind the buffer depending on the usage
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