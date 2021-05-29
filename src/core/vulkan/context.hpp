#pragma once
#include "../settings.hpp"
#include "../window.hpp"
#include "render_call.hpp"
#include "swap_chain.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {

class Context {
public:
  class QueueFamilyIndices {
  public:
    QueueFamilyIndices(const vk::PhysicalDevice &device,
                       const vk::SurfaceKHR &surface);

    inline bool is_complete() const {
      return graphics_family.has_value() && present_family.has_value();
    }

    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
  };

  class SwapChainSupportDetails {
  public:
    SwapChainSupportDetails(const vk::PhysicalDevice &device,
                            const vk::SurfaceKHR &surface);

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
  };

  class PhysicalDeviceInfo {
  public:
    PhysicalDeviceInfo(const vk::PhysicalDevice &device,
                       const vk::SurfaceKHR &surface);

    const vk::PhysicalDeviceProperties properties;
    const vk::Format depth_format;
    const QueueFamilyIndices queue_family_indices;
    const vk::SampleCountFlagBits max_msaa_samples;
    SwapChainSupportDetails swap_chain_support_details;
    bool linear_blitting_support;

  private:
    static vk::Format
    _find_supported_format(const vk::PhysicalDevice &device,
                           const std::vector<vk::Format> &candidates,
                           vk::ImageTiling tiling,
                           vk::FormatFeatureFlags features);
    static inline vk::Format
    _find_depth_format(const vk::PhysicalDevice &device) {
      return _find_supported_format(
          device,
          {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
           vk::Format::eD24UnormS8Uint},
          vk::ImageTiling::eOptimal,
          vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
    static vk::SampleCountFlagBits
    _get_max_usable_sample_count(const vk::PhysicalDeviceProperties &props);
  };

  friend class SwapChain;
  friend class RenderCall;

  static PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
  static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

  Context(Window &window, Settings &settings);
  ~Context();

  // ***** inline methods *****
  inline size_t get_swap_chain_image_count() const {
    return m_swap_chain->get_image_count();
  }
  inline const vk::RenderPass &get_swap_chain_render_pass() const {
    return m_swap_chain->get_render_pass();
  }
  inline const vk::Device &get_device() const noexcept { return m_device; }
  inline const PhysicalDeviceInfo &get_physical_device_info() const noexcept {
    return *m_physical_device_info;
  }
  inline vk::CommandBuffer begin_single_time_graphics_commands() const {
    return _begin_single_time_commands(m_device, m_graphic_command_pool);
  }
  inline void
  end_single_time_graphics_commands(vk::CommandBuffer buffer) const {
    _end_single_time_commands(m_device, m_graphic_command_pool,
                              m_graphics_queue, std::move(buffer));
  }
  // ***************************

  // **** utility methods *******
  void transition_image_layout(const vk::Image &image, vk::Format format,
                               vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               uint32_t mip_levels) const;
  uint32_t find_memory_type(uint32_t type_filter,
                            vk::MemoryPropertyFlags props) const;
  // ****************************

  // ***** render methods ********
  std::optional<RenderCall> render_begin();
  // *****************************

private:
  // ****** constants **************
  static constexpr char _application_name[] = "VulkanKraft";
  static constexpr uint32_t _application_version =
      VK_MAKE_API_VERSION(0, 0, 0, 0);
  static constexpr char _engine_name[] = "VulkanKraft Core";
  static constexpr uint32_t _engine_version = VK_MAKE_API_VERSION(0, 0, 0, 0);
#ifdef NDEBUG
  static constexpr bool _enable_validation_layers = false;
#else
  static constexpr bool _enable_validation_layers = true;
#endif
  static constexpr size_t _max_images_in_flight = 2;
  // *********************************

  // ******* validation layers *******
  static void _populate_debug_messenger_create_info(
      vk::DebugUtilsMessengerCreateInfoEXT &di) noexcept;
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  _debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData);
  // *********************************

  // ****** Utilities ****************

  static bool
  _is_device_suitable(const vk::PhysicalDevice &device,
                      const vk::SurfaceKHR &surface,
                      const std::vector<const char *> &extension_names);
  static bool _device_has_extension_support(
      const vk::PhysicalDevice &device,
      const std::vector<const char *> &extension_names);
  static bool _has_validation_layer_support(
      const std::vector<const char *> &layer_names) noexcept;

  static vk::CommandBuffer
  _begin_single_time_commands(const vk::Device &device,
                              const vk::CommandPool &command_pool);
  static void _end_single_time_commands(const vk::Device &device,
                                        const vk::CommandPool &command_pool,
                                        const vk::Queue &queue,
                                        vk::CommandBuffer command_buffer);
  static inline constexpr bool _has_stencil_component(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint ||
           format == vk::Format::eD24UnormS8Uint;
  }
  // ****************************

  // ****** Initialisation ******
  void _create_instance(const Window &window,
                        const std::vector<const char *> &validation_layers);
  void _setup_debug_messenger();
  void _create_surface(const Window &window);
  void _pick_physical_device(const std::vector<const char *> &extensions);
  void
  _create_logical_device(const std::vector<const char *> &extensions,
                         const std::vector<const char *> &validation_layers);
  void _create_command_pool();
  void _create_swap_chain(const Window &window);
  void _allocate_command_buffers();
  void _create_sync_objects();
  // ****************************

  void _handle_framebuffer_resize();

  vk::Instance m_instance;
  vk::DebugUtilsMessengerEXT m_debug_messenger;
  vk::SurfaceKHR m_surface;
  vk::PhysicalDevice m_physical_device;
  vk::Device m_device;
  vk::Queue m_graphics_queue;
  vk::Queue m_present_queue;
  std::unique_ptr<SwapChain> m_swap_chain;
  vk::CommandPool m_graphic_command_pool;
  std::vector<vk::CommandBuffer> m_graphic_command_buffers;

  std::vector<vk::Semaphore> m_image_available_semaphores;
  std::vector<vk::Semaphore> m_render_finished_semaphores;
  std::vector<vk::Fence> m_in_flight_fences;
  std::vector<vk::Fence> m_images_in_flight;

  size_t m_current_frame;
  bool m_framebuffer_resized;
  std::unique_ptr<PhysicalDeviceInfo> m_physical_device_info;
  const Settings &m_settings;
};
} // namespace vulkan
} // namespace core