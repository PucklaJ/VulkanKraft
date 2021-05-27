#include "buffer.hpp"
#include "../exception.hpp"
#include <cstring>

namespace core {
namespace vulkan {
Buffer::Buffer(const Context &context, vk::BufferUsageFlags usage,
               const size_t buffer_size, const void *data)
    : m_usage(usage | ((usage & vk::BufferUsageFlagBits::eUniformBuffer)
                           ? static_cast<vk::BufferUsageFlagBits>(0)
                           : vk::BufferUsageFlagBits::eTransferDst)),
      m_context(context) {
  vk::BufferCreateInfo bi;
  bi.size = static_cast<vk::DeviceSize>(buffer_size);
  bi.usage = m_usage;
  bi.sharingMode = vk::SharingMode::eExclusive;

  try {
    m_handle = m_context.m_device.createBuffer(bi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create buffer: ") +
                               e.what());
  }

  const auto mem_req = m_context.m_device.getBufferMemoryRequirements(m_handle);
  vk::MemoryAllocateInfo ai;
  ai.allocationSize = mem_req.size;
  ai.memoryTypeIndex = Context::find_memory_type(
      m_context.m_physical_device, mem_req.memoryTypeBits,
      (m_usage & vk::BufferUsageFlagBits::eUniformBuffer)
          ? (vk::MemoryPropertyFlagBits::eHostVisible |
             vk::MemoryPropertyFlagBits::eHostCoherent)
          : vk::MemoryPropertyFlagBits::eDeviceLocal);
  try {
    m_memory = m_context.m_device.allocateMemory(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to allocate memory for buffer: ") + e.what());
  }

  m_context.m_device.bindBufferMemory(m_handle, m_memory, 0);

  if (data) {
    set_data(data, buffer_size);
  }
}

Buffer::~Buffer() {
  m_context.m_device.waitIdle();

  m_context.m_device.destroyBuffer(m_handle);
  m_context.m_device.freeMemory(m_memory);
}

void Buffer::set_data(const void *data, const size_t data_size,
                      const size_t offset) {
  if (m_usage & vk::BufferUsageFlagBits::eUniformBuffer) {
    // memcpy data directly to buffer
    try {
      void *mapped_data = m_context.m_device.mapMemory(
          m_memory, 0, static_cast<vk::DeviceSize>(data_size));
      memcpy(mapped_data, data, data_size);
      m_context.m_device.unmapMemory(m_memory);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string("failed to map memory of uniform buffer: ") + e.what());
    }
  } else {

    vk::Buffer staging_buffer;
    vk::DeviceMemory staging_memory;

    // Create staging buffer
    vk::BufferCreateInfo bi;
    bi.size = static_cast<vk::DeviceSize>(data_size);
    bi.usage = vk::BufferUsageFlagBits::eTransferSrc;
    bi.sharingMode = vk::SharingMode::eExclusive;

    try {
      staging_buffer = m_context.m_device.createBuffer(bi);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string("failed to create staging buffer: ") + e.what());
    }

    // Create Staging buffer memory
    const auto mem_req =
        m_context.m_device.getBufferMemoryRequirements(staging_buffer);
    vk::MemoryAllocateInfo ai;
    ai.allocationSize = mem_req.size;
    ai.memoryTypeIndex = Context::find_memory_type(
        m_context.m_physical_device, mem_req.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
    try {
      staging_memory = m_context.m_device.allocateMemory(ai);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string("failed to allocate memory for staging buffer: ") +
          e.what());
    }

    m_context.m_device.bindBufferMemory(staging_buffer, staging_memory, 0);

    // memcpy data to staging buffer
    try {
      void *mapped_data = m_context.m_device.mapMemory(
          staging_memory, 0, static_cast<vk::DeviceSize>(data_size));
      memcpy(mapped_data, data, data_size);
      m_context.m_device.unmapMemory(staging_memory);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string("failed to map memory of staging buffer: ") + e.what());
    }

    // Copy staging buffer to buffer
    auto com_buf = Context::begin_single_time_commands(
        m_context.m_device, m_context.m_graphic_command_pool);

    vk::BufferCopy cp;
    cp.srcOffset = 0;
    cp.dstOffset = static_cast<vk::DeviceSize>(offset);
    cp.size = static_cast<vk::DeviceSize>(data_size);
    com_buf.copyBuffer(staging_buffer, m_handle, cp);

    Context::end_single_time_commands(
        m_context.m_device, m_context.m_graphic_command_pool,
        m_context.m_graphics_queue, std::move(com_buf));

    m_context.m_device.destroyBuffer(staging_buffer);
    m_context.m_device.freeMemory(staging_memory);
  }
}

void Buffer::bind(const RenderCall &render_call) const {
  render_call.bind_buffer(m_handle, m_usage);
}

} // namespace vulkan
} // namespace core
