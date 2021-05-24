#pragma once
#include "../window.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class SwapChain {
public:
  SwapChain(const vk::PhysicalDevice &physical_device, const vk::Device &device,
            const vk::SurfaceKHR &surface, const vk::RenderPass &render_pass,
            const Window &window);
  ~SwapChain();

  inline const vk::Format &get_image_format() const { return m_image_format; }
  inline const vk::Extent2D &get_extent() const { return m_extent; }
  void create_framebuffers(vk::ImageView &depth_image_view);

private:
  static vk::SurfaceFormatKHR
  _choose_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats);
  static vk::PresentModeKHR
  _choose_present_mode(const std::vector<vk::PresentModeKHR> &present_modes);
  static vk::Extent2D _choose_extent(const vk::SurfaceCapabilitiesKHR &caps,
                                     const Window &window);

  void _create_image_views();
  void _destroy();

  vk::SwapchainKHR m_handle;
  vk::Format m_image_format;
  vk::Extent2D m_extent;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_image_views;
  std::vector<vk::Framebuffer> m_framebuffers;

  const vk::Device &m_device;
  const vk::RenderPass &m_render_pass;
};
} // namespace vulkan
} // namespace core