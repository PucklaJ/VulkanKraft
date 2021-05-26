#pragma once
#include "context.hpp"

namespace core {
namespace vulkan {
class Buffer {
public:
  Buffer(const Context &context, vk::BufferUsageFlags usage,
         const size_t buffer_size, const void *data = nullptr);
  ~Buffer();

  void set_data(const void *data, const size_t data_size,
                const size_t offset = 0);
  void bind();

private:
  vk::Buffer m_handle;
  vk::DeviceMemory m_memory;
  const vk::BufferUsageFlags m_usage;

  const Context &m_context;
};
} // namespace vulkan
} // namespace core