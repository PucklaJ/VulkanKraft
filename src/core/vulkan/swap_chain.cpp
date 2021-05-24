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
                     const Window &window)
    : m_device(device) {
  const Context::SwapChainSupportDetails scs(physical_device, surface);
  const auto format{_choose_surface_format(scs.formats)};
  const auto present_mode{_choose_present_mode(scs.present_modes)};
  const auto extent{_choose_extent(scs.capabilities, window)};

  auto image_count{scs.capabilities.minImageCount + 1};
  if (scs.capabilities.minImageCount > 0 &&
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
}

SwapChain::~SwapChain() {}

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

void SwapChain::_destroy() { m_device.destroySwapchainKHR(m_handle); }

} // namespace vulkan
} // namespace core