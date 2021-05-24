#pragma once
#include "../window.hpp"
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class SwapChain {
public:
  SwapChain(const vk::PhysicalDevice &physical_device, const vk::Device &device,
            const vk::SurfaceKHR &surface, const Window &window);
  ~SwapChain();

private:
  static vk::SurfaceFormatKHR
  _choose_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats);
  static vk::PresentModeKHR
  _choose_present_mode(const std::vector<vk::PresentModeKHR> &present_modes);
  static vk::Extent2D _choose_extent(const vk::SurfaceCapabilitiesKHR &caps,
                                     const Window &window);

  void _destroy();

  vk::SwapchainKHR m_handle;
  vk::Format m_image_format;
  vk::Extent2D m_extent;
  std::vector<vk::Image> m_images;
  const vk::Device &m_device;
};
} // namespace vulkan
} // namespace core