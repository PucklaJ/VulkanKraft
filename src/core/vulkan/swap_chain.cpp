#include "swap_chain.hpp"
#include "../exception.hpp"
#include "context.hpp"
#include <algorithm>
#include <array>
#include <limits>

namespace core {
namespace vulkan {
SwapChain::SwapChain(const vk::PhysicalDevice &physical_device,
                     const vk::Device &device, const vk::SurfaceKHR &surface,
                     const vk::RenderPass &render_pass, const Window &window)
    : m_device(device), m_render_pass(render_pass) {
  const Context::SwapChainSupportDetails scs(physical_device, surface);
  const auto format{_choose_surface_format(scs.formats)};
  const auto present_mode{_choose_present_mode(scs.present_modes)};
  const auto extent{_choose_extent(scs.capabilities, window)};

  auto image_count{scs.capabilities.minImageCount + 1};
  if (scs.capabilities.maxImageCount > 0 &&
      image_count > scs.capabilities.maxImageCount) {
    image_count = scs.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR si;
  si.surface = surface;
  si.minImageCount = image_count;
  si.imageFormat = format.format;
  si.imageColorSpace = format.colorSpace;
  si.imageExtent = extent;
  si.imageArrayLayers = 1;
  si.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  const Context::QueueFamilyIndices indices(physical_device, surface);
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
    m_handle = device.createSwapchainKHR(si);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create swap chain: ") +
                               e.what());
  }

  m_images = device.getSwapchainImagesKHR(m_handle);
  m_image_format = format.format;
  m_extent = extent;

  _create_image_views();
}

SwapChain::~SwapChain() { _destroy(); }

void SwapChain::create_framebuffers() {
  m_framebuffers.reserve(m_image_views.size());
  for (const auto &iv : m_image_views) {
    vk::FramebufferCreateInfo fb_i;
    fb_i.renderPass = m_render_pass;
    fb_i.attachmentCount = 1;
    fb_i.pAttachments = &iv;
    fb_i.width = m_extent.width;
    fb_i.height = m_extent.height;
    fb_i.layers = 1;

    try {
      m_framebuffers.emplace_back(m_device.createFramebuffer(fb_i));
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException("failed to create swap chain framebuffer " +
                                 std::to_string(m_framebuffers.size()) + ": " +
                                 e.what());
    }
  }
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
      m_image_views.emplace_back(m_device.createImageView(vi));
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(std::string("failed to create image view ") +
                                 std::to_string(m_image_views.size()) + ": " +
                                 e.what());
    }
  }
}

void SwapChain::_destroy() {
  for (auto &fb : m_framebuffers) {
    m_device.destroyFramebuffer(fb);
  }
  for (auto &iv : m_image_views) {
    m_device.destroyImageView(iv);
  }
  m_image_views.clear();
  m_device.destroySwapchainKHR(m_handle);
}

} // namespace vulkan
} // namespace core