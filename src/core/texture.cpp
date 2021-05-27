#include "texture.hpp"
#include "exception.hpp"

namespace core {
Texture::Builder::Builder()
    : m_width(0), m_height(0), m_filter(vk::Filter::eLinear),
      m_address_mode(vk::SamplerAddressMode::eRepeat), m_max_anisotropy(0.0f),
      m_border_color(vk::BorderColor::eIntOpaqueBlack) {}

Texture Texture::Builder::build(const vulkan::Context &context,
                                const void *data) {
  return Texture(context, *this, data);
}

Texture::~Texture() {
  m_context.destroy_texture(std::move(m_image), std::move(m_image_view),
                            std::move(m_memory), std::move(m_sampler));
}

Texture::Texture(const vulkan::Context &context,
                 const Texture::Builder &builder, const void *data)
    : m_context(context) {
  _create_image(builder, data);
  _create_image_view(builder);
  _create_sampler(builder);
}

void Texture::_create_image(const Texture::Builder &builder, const void *data) {
  // Create Image
  vk::ImageCreateInfo ii;
  ii.imageType = vk::ImageType::e2D;
  ii.extent.width = builder.m_width;
  ii.extent.height = builder.m_height;
  ii.extent.depth = 1;
  ii.mipLevels = 1;
  ii.arrayLayers = 1;
  ii.format = vk::Format::eR8G8B8A8Srgb;
  ii.tiling = vk::ImageTiling::eOptimal;
  ii.initialLayout = vk::ImageLayout::eUndefined;
  ii.usage =
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
  ii.samples = vk::SampleCountFlagBits::e1;
  ii.sharingMode = vk::SharingMode::eExclusive;

  try {
    m_image = m_context.get_device().createImage(ii);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create image of core::Texture: ") + e.what());
  }

  // Allocate Memory
  auto mem_req(m_context.get_device().getImageMemoryRequirements(m_image));

  vk::MemoryAllocateInfo ai;
  ai.allocationSize = mem_req.size;
  ai.memoryTypeIndex = vulkan::Context::find_memory_type(
      m_context.get_physical_device(), mem_req.memoryTypeBits,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  try {
    m_memory = m_context.get_device().allocateMemory(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to allocate memory for core::Texture: ") +
        e.what());
  }

  m_context.get_device().bindImageMemory(m_image, m_memory, 0);

  const auto image_size{builder.m_width * builder.m_height * 4};

  // Create Staging Buffer
  vk::BufferCreateInfo bi;
  bi.size = image_size;
  bi.usage = vk::BufferUsageFlagBits::eTransferSrc;
  bi.sharingMode = vk::SharingMode::eExclusive;

  vk::Buffer staging_buffer;
  try {
    staging_buffer = m_context.get_device().createBuffer(bi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create staging buffer for core::Texture: ") +
        e.what());
  }

  mem_req = m_context.get_device().getBufferMemoryRequirements(staging_buffer);

  ai.allocationSize = mem_req.size;
  ai.memoryTypeIndex = vulkan::Context::find_memory_type(
      m_context.get_physical_device(), mem_req.memoryTypeBits,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  vk::DeviceMemory staging_buffer_memory;
  try {
    staging_buffer_memory = m_context.get_device().allocateMemory(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to allocate memory for staging buffer of core::Texture: ") +
        e.what());
  }

  m_context.get_device().bindBufferMemory(staging_buffer, staging_buffer_memory,
                                          0);

  // Write data to staging buffer
  try {
    void *mapped_data =
        m_context.get_device().mapMemory(staging_buffer_memory, 0, image_size);
    memcpy(mapped_data, data, image_size);
    m_context.get_device().unmapMemory(staging_buffer_memory);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to write to staging buffer of core::Texture: ") +
        e.what());
  }

  // Transistion image layout
  m_context.transition_image_layout(m_image, vk::Format::eR8G8B8A8Srgb,
                                    vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal, 1);

  // Copy staging buffer contents to image
  try {
    auto com_buf = m_context.begin_single_time_graphics_commands();

    vk::BufferImageCopy cp;
    cp.bufferOffset = 0;
    cp.bufferRowLength = 0;
    cp.bufferImageHeight = 0;

    cp.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    cp.imageSubresource.mipLevel = 0;
    cp.imageSubresource.baseArrayLayer = 0;
    cp.imageSubresource.layerCount = 1;

    cp.imageOffset = vk::Offset3D{0, 0, 0};
    cp.imageExtent = vk::Extent3D{builder.m_width, builder.m_height, 1};

    com_buf.copyBufferToImage(staging_buffer, m_image,
                              vk::ImageLayout::eTransferDstOptimal, cp);

    m_context.end_single_time_graphics_commands(std::move(com_buf));
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to copy staging buffer to image of core::Texture: ") +
        e.what());
  }

  // Destroy staging buffer
  m_context.get_device().destroyBuffer(staging_buffer);
  m_context.get_device().freeMemory(staging_buffer_memory);
}

void Texture::_create_image_view(const Texture::Builder &builder) {
  vk::ImageViewCreateInfo vi;
  vi.image = m_image;
  vi.viewType = vk::ImageViewType::e2D;
  vi.format = vk::Format::eR8G8B8A8Srgb;
  vi.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  vi.subresourceRange.baseMipLevel = 0;
  vi.subresourceRange.levelCount = 1;
  vi.subresourceRange.baseArrayLayer = 0;
  vi.subresourceRange.layerCount = 1;

  try {
    m_image_view = m_context.get_device().createImageView(vi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create image view of core::Texture: ") +
        e.what());
  }
}

void Texture::_create_sampler(const Texture::Builder &builder) {
  throw VulkanKraftException(__FUNCTION__ + std::string(" not implemented"));
}
} // namespace core
