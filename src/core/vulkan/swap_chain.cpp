#include "swap_chain.hpp"
#include "../exception.hpp"
#include "context.hpp"
#include <algorithm>
#include <array>
#include <limits>

namespace core {
namespace vulkan {
SwapChain::SwapChain(const Context *context, const Window &window,
                     const vk::SampleCountFlagBits msaa_samples)
    : m_context(context), m_window(window) {
  _create_handle();
  _retrieve_images();
  _create_image_views();
  _create_render_pass(msaa_samples);
  _create_color_image(msaa_samples);
  _create_depth_image(msaa_samples);
  _create_framebuffers();
}

SwapChain::~SwapChain() { _destroy(); }

std::optional<std::tuple<uint32_t, vk::Framebuffer>>
SwapChain::acquire_next_image(const vk::Device &m_device,
                              const vk::Semaphore &semaphore) {
  try {
    const auto r = m_device.acquireNextImageKHR(
        m_handle, std::numeric_limits<uint64_t>::max(), semaphore,
        VK_NULL_HANDLE);
    switch (r.result) {
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
      m_current_image = r.value;
      return std::make_tuple(r.value, m_framebuffers[r.value]);
    case vk::Result::eErrorOutOfDateKHR:
      m_current_image = std::nullopt;
      return std::nullopt;
    default:
      throw std::runtime_error(std::to_string(static_cast<int>(r.result)));
    }
  } catch (const vk::OutOfDateKHRError &e) {
    m_current_image = std::nullopt;
    return std::nullopt;
  } catch (const std::runtime_error &e) {
    m_current_image = std::nullopt;
    throw VulkanKraftException(
        std::string("failed to acquire next swap chain image: ") + e.what());
  }
}

void SwapChain::recreate(const vk::SampleCountFlagBits msaa_samples) {
  _destroy(false);

  _create_handle();
  _retrieve_images();
  _create_image_views();
  _create_color_image(msaa_samples);
  _create_depth_image(msaa_samples);
  _create_framebuffers();
}

vk::SurfaceFormatKHR SwapChain::_choose_surface_format(
    const std::vector<vk::SurfaceFormatKHR> &formats) {
  for (const auto &f : formats) {
    if (f.format == vk::Format::eB8G8R8A8Srgb) {
      return f;
    }
  }

  return formats[0];
}

vk::PresentModeKHR SwapChain::_choose_present_mode(
    const std::vector<vk::PresentModeKHR> &present_modes) {
  for (const auto &pm : present_modes) {
    if (pm == vk::PresentModeKHR::eMailbox) {
      return pm;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::_choose_extent(const vk::SurfaceCapabilitiesKHR &caps,
                                       const Window &window) {
  if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return caps.currentExtent;
  }

  const auto [width, height] = window.get_framebuffer_size();
  return vk::Extent2D(std::max(caps.minImageExtent.width,
                               std::min(caps.maxImageExtent.width, width)),
                      std::max(caps.minImageExtent.height,
                               std::min(caps.maxImageExtent.height, height)));
}

void SwapChain::_create_handle() {
  const auto &scs =
      m_context->get_physical_device_info().swap_chain_support_details;
  const auto format{_choose_surface_format(scs.formats)};
  const auto present_mode{_choose_present_mode(scs.present_modes)};
  const auto extent{_choose_extent(scs.capabilities, m_window)};

  auto image_count{scs.capabilities.minImageCount + 1};
  if (scs.capabilities.maxImageCount > 0 &&
      image_count > scs.capabilities.maxImageCount) {
    image_count = scs.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR si;
  si.surface = m_context->m_surface;
  si.minImageCount = image_count;
  si.imageFormat = format.format;
  si.imageColorSpace = format.colorSpace;
  si.imageExtent = extent;
  si.imageArrayLayers = 1;
  si.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  const auto &indices =
      m_context->get_physical_device_info().queue_family_indices;
  const auto qfi = std::array{indices.graphics_family.value(),
                              indices.present_family.value()};
  if (indices.graphics_family != indices.present_family) {
    si.imageSharingMode = vk::SharingMode::eConcurrent;
    si.queueFamilyIndexCount = 2;
    si.pQueueFamilyIndices = qfi.data();
  } else {
    si.imageSharingMode = vk::SharingMode::eExclusive;
  }

  si.preTransform = scs.capabilities.currentTransform;
  si.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  si.presentMode = present_mode;
  si.clipped = VK_TRUE;
  si.oldSwapchain = VK_NULL_HANDLE;

  try {
    m_handle = m_context->m_device.createSwapchainKHR(si);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create swap chain: ") +
                               e.what());
  }

  m_image_format = format.format;
  m_extent = extent;
}

void SwapChain::_retrieve_images() {
  m_images = m_context->m_device.getSwapchainImagesKHR(m_handle);
}

void SwapChain::_create_image_views() {
  vk::ImageViewCreateInfo vi;
  vi.viewType = vk::ImageViewType::e2D;
  vi.format = m_image_format;
  vi.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  vi.subresourceRange.baseMipLevel = 0;
  vi.subresourceRange.levelCount = 1;
  vi.subresourceRange.baseArrayLayer = 0;
  vi.subresourceRange.layerCount = 1;

  m_image_views.reserve(m_images.size());
  for (const auto &i : m_images) {
    vi.image = i;
    try {
      m_image_views.emplace_back(m_context->m_device.createImageView(vi));
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(std::string("failed to create image view ") +
                                 std::to_string(m_image_views.size()) + ": " +
                                 e.what());
    }
  }
}

void SwapChain::_create_render_pass(
    const vk::SampleCountFlagBits msaa_samples) {
  vk::AttachmentDescription col_at;
  col_at.format = m_image_format;
  col_at.samples = msaa_samples;
  col_at.loadOp = vk::AttachmentLoadOp::eClear;
  col_at.storeOp = vk::AttachmentStoreOp::eStore;
  col_at.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  col_at.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  col_at.initialLayout = vk::ImageLayout::eUndefined;
  col_at.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentDescription depth_at;
  depth_at.format = m_context->get_physical_device_info().depth_format;
  depth_at.samples = msaa_samples;
  depth_at.loadOp = vk::AttachmentLoadOp::eClear;
  depth_at.storeOp = vk::AttachmentStoreOp::eDontCare;
  depth_at.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  depth_at.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  depth_at.initialLayout = vk::ImageLayout::eUndefined;
  depth_at.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::AttachmentDescription col_res_at;
  col_res_at.format = m_image_format;
  col_res_at.samples = vk::SampleCountFlagBits::e1;
  col_res_at.loadOp = vk::AttachmentLoadOp::eDontCare;
  col_res_at.storeOp = vk::AttachmentStoreOp::eStore;
  col_res_at.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  col_res_at.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  col_res_at.initialLayout = vk::ImageLayout::eUndefined;
  col_res_at.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference col_at_ref;
  col_at_ref.attachment = 0;
  col_at_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::AttachmentReference depth_at_ref;
  depth_at_ref.attachment = 1;
  depth_at_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::AttachmentReference col_res_at_ref;
  col_res_at_ref.attachment = 2;
  col_res_at_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription sub;
  sub.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  sub.colorAttachmentCount = 1;
  sub.pColorAttachments = &col_at_ref;
  sub.pDepthStencilAttachment = &depth_at_ref;
  sub.pResolveAttachments = &col_res_at_ref;

  vk::SubpassDependency dep;
  dep.srcSubpass = VK_SUBPASS_EXTERNAL;
  dep.dstSubpass = 0;
  dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                     vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dep.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
  dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                     vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                      vk::AccessFlagBits::eDepthStencilAttachmentWrite;

  const auto ats = std::array{col_at, depth_at, col_res_at};
  vk::RenderPassCreateInfo ri;
  ri.attachmentCount = static_cast<uint32_t>(ats.size());
  ri.pAttachments = ats.data();
  ri.subpassCount = 1;
  ri.pSubpasses = &sub;
  ri.dependencyCount = 1;
  ri.pDependencies = &dep;

  try {
    m_render_pass = m_context->m_device.createRenderPass(ri);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create render pass: ") +
                               e.what());
  }
}

void SwapChain::_create_color_image(
    const vk::SampleCountFlagBits msaa_samples) {
  // Create image
  vk::ImageCreateInfo ii;
  ii.imageType = vk::ImageType::e2D;
  ii.extent.width = m_extent.width;
  ii.extent.height = m_extent.height;
  ii.extent.depth = 1;
  ii.mipLevels = 1;
  ii.arrayLayers = 1;
  ii.format = m_image_format;
  ii.tiling = vk::ImageTiling::eOptimal;
  ii.initialLayout = vk::ImageLayout::eUndefined;
  ii.usage = vk::ImageUsageFlagBits::eTransientAttachment |
             vk::ImageUsageFlagBits::eColorAttachment;
  ii.samples = msaa_samples;
  ii.sharingMode = vk::SharingMode::eExclusive;

  try {
    m_color_image = m_context->get_device().createImage(ii);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to create color image of core::vulkan::SwapChain: ") +
        e.what());
  }

  // Allocate memory
  const auto mem_req =
      m_context->get_device().getImageMemoryRequirements(m_color_image);
  vk::MemoryAllocateInfo ai;
  ai.allocationSize = mem_req.size;
  ai.memoryTypeIndex = m_context->find_memory_type(
      mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
  try {
    m_color_image_memory = m_context->get_device().allocateMemory(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to allocate memory for color image of "
                    "core::vulkan::SwapChain: ") +
        e.what());
  }

  m_context->get_device().bindImageMemory(m_color_image, m_color_image_memory,
                                          0);

  // Create Image View
  vk::ImageViewCreateInfo vi;
  vi.image = m_color_image;
  vi.viewType = vk::ImageViewType::e2D;
  vi.format = m_image_format;
  vi.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  vi.subresourceRange.baseMipLevel = 0;
  vi.subresourceRange.levelCount = 1;
  vi.subresourceRange.baseArrayLayer = 0;
  vi.subresourceRange.layerCount = 1;

  try {
    m_color_image_view = m_context->get_device().createImageView(vi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to create color image view of core::vulkan::SwapChain: ") +
        e.what());
  }
}

void SwapChain::_create_depth_image(
    const vk::SampleCountFlagBits msaa_samples) {
  const auto format{m_context->get_physical_device_info().depth_format};

  // Create image
  vk::ImageCreateInfo ii{};
  ii.imageType = vk::ImageType::e2D;
  ii.extent.width = m_extent.width;
  ii.extent.height = m_extent.height;
  ii.extent.depth = 1;
  ii.mipLevels = 1;
  ii.arrayLayers = 1;
  ii.format = format;
  ii.tiling = vk::ImageTiling::eOptimal;
  ii.initialLayout = vk::ImageLayout::eUndefined;
  ii.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
  ii.samples = msaa_samples;
  ii.sharingMode = vk::SharingMode::eExclusive;

  try {
    m_depth_image = m_context->m_device.createImage(ii);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create depth image: ") +
                               e.what());
  }

  // Allocate memory
  const auto mem_req{
      m_context->m_device.getImageMemoryRequirements(m_depth_image)};

  vk::MemoryAllocateInfo ai;
  ai.allocationSize = mem_req.size;
  ai.memoryTypeIndex = m_context->find_memory_type(
      mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

  try {
    m_depth_image_memory = m_context->m_device.allocateMemory(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to allocate depth image memory: ") + e.what());
  }

  m_context->m_device.bindImageMemory(m_depth_image, m_depth_image_memory, 0);

  // Create image view
  vk::ImageViewCreateInfo vi{};
  vi.image = m_depth_image;
  vi.viewType = vk::ImageViewType::e2D;
  vi.format = format;
  vi.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
  vi.subresourceRange.baseMipLevel = 0;
  vi.subresourceRange.levelCount = 1;
  vi.subresourceRange.baseArrayLayer = 0;
  vi.subresourceRange.layerCount = 1;

  VkImageView imageView;
  try {
    m_depth_image_view = m_context->m_device.createImageView(vi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create depth image view: ") + e.what());
  }

  m_context->transition_image_layout(
      m_depth_image, format, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
}

void SwapChain::_create_framebuffers() {
  m_framebuffers.reserve(m_image_views.size());
  for (const auto &iv : m_image_views) {
    const auto ats = std::array{m_color_image_view, m_depth_image_view, iv};

    vk::FramebufferCreateInfo fb_i;
    fb_i.renderPass = m_render_pass;
    fb_i.attachmentCount = static_cast<uint32_t>(ats.size());
    fb_i.pAttachments = ats.data();
    fb_i.width = m_extent.width;
    fb_i.height = m_extent.height;
    fb_i.layers = 1;

    try {
      m_framebuffers.emplace_back(m_context->m_device.createFramebuffer(fb_i));
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException("failed to create swap chain framebuffer " +
                                 std::to_string(m_framebuffers.size()) + ": " +
                                 e.what());
    }
  }
}

void SwapChain::_destroy(const bool everything) {
  m_context->m_device.destroyImageView(m_depth_image_view);
  m_context->m_device.destroyImage(m_depth_image);
  m_context->m_device.freeMemory(m_depth_image_memory);
  m_context->m_device.destroyImageView(m_color_image_view);
  m_context->m_device.destroyImage(m_color_image);
  m_context->m_device.freeMemory(m_color_image_memory);
  for (auto &fb : m_framebuffers) {
    m_context->m_device.destroyFramebuffer(fb);
  }
  m_framebuffers.clear();
  if (everything) {
    m_context->m_device.destroyRenderPass(m_render_pass);
  }
  for (auto &iv : m_image_views) {
    m_context->m_device.destroyImageView(iv);
  }
  m_image_views.clear();
  m_context->m_device.destroySwapchainKHR(m_handle);
}

} // namespace vulkan
} // namespace core