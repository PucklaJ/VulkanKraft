#pragma once
#include "../window.hpp"
#include <optional>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class Context;

class SwapChain {
public:
  SwapChain(const Context *context, const Window &window);
  ~SwapChain();

  inline const vk::Format &get_image_format() const { return m_image_format; }
  inline const vk::Extent2D &get_extent() const { return m_extent; }
  inline size_t get_image_count() const { return m_images.size(); }
  inline vk::SwapchainKHR &get_handle() { return m_handle; }
  inline const std::optional<uint32_t> &get_current_image() const {
    return m_current_image;
  }
  inline const vk::RenderPass &get_render_pass() const { return m_render_pass; }
  std::optional<std::tuple<uint32_t, vk::Framebuffer>>
  acquire_next_image(const vk::Device &device, const vk::Semaphore &semaphore);
  void recreate();

private:
  static vk::SurfaceFormatKHR
  _choose_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats);
  static vk::PresentModeKHR
  _choose_present_mode(const std::vector<vk::PresentModeKHR> &present_modes);
  static vk::Extent2D _choose_extent(const vk::SurfaceCapabilitiesKHR &caps,
                                     const Window &window);

  // ***** Intialisation *****
  void _create_handle();
  void _retrieve_images();
  void _create_image_views();
  void _create_render_pass();
  void _create_depth_image();
  void _create_framebuffers();
  // **************************
  void _destroy(const bool everything = true);

  vk::SwapchainKHR m_handle;
  vk::Format m_image_format;
  vk::Extent2D m_extent;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_image_views;
  std::vector<vk::Framebuffer> m_framebuffers;
  std::optional<uint32_t> m_current_image;
  vk::RenderPass m_render_pass;
  vk::Image m_depth_image;
  vk::DeviceMemory m_depth_image_memory;
  vk::ImageView m_depth_image_view;

  const Context *m_context;
  const Window &m_window;
};
} // namespace vulkan
} // namespace core