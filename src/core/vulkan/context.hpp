#pragma once
#include "../window.hpp"
#include "swap_chain.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class Context {
public:
  friend class SwapChain;
  friend class GraphicsPipeline;

  static PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
  static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

  Context(const Window &window);
  ~Context();

  void render_begin();
  void render_end();
  void render_vertices(const uint32_t num_vertices,
                       const uint32_t first_vertex = 0);

private:
  class QueueFamilyIndices {
  public:
    QueueFamilyIndices(const vk::PhysicalDevice &device,
                       const vk::SurfaceKHR &surface);

    bool is_complete() const;

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

  static bool _has_validation_layer_support(
      const std::vector<const char *> &layer_names) noexcept;
  static void _populate_debug_messenger_create_info(
      vk::DebugUtilsMessengerCreateInfoEXT &di) noexcept;
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  _debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData);
  static bool
  _is_device_suitable(const vk::PhysicalDevice &device,
                      const vk::SurfaceKHR &surface,
                      const std::vector<const char *> &extension_names);
  static bool _device_has_extension_support(
      const vk::PhysicalDevice &device,
      const std::vector<const char *> &extension_names);

  // ****** Utilities ***********
  static vk::Format
  _find_supported_format(const vk::PhysicalDevice &device,
                         const std::vector<vk::Format> &candidates,
                         vk::ImageTiling tiling,
                         vk::FormatFeatureFlags features);
  static vk::Format _find_depth_format(const vk::PhysicalDevice &device);
  static uint32_t _find_memory_type(const vk::PhysicalDevice &device,
                                    uint32_t type_filter,
                                    vk::MemoryPropertyFlags props);
  static vk::CommandBuffer
  _begin_single_time_commands(const vk::Device &device,
                              const vk::CommandPool &command_pool);
  static void _end_single_time_commands(const vk::Device &device,
                                        const vk::CommandPool &command_pool,
                                        const vk::Queue &queue,
                                        vk::CommandBuffer command_buffer);
  static void
  _transition_image_layout(const vk::Device &device,
                           const vk::CommandPool &command_pool,
                           const vk::Queue &queue, const vk::Image &image,
                           vk::Format format, vk::ImageLayout old_layout,
                           vk::ImageLayout new_layout, uint32_t mip_levels);
  static inline bool _has_stencil_component(vk::Format format) {
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
  void _create_swap_chain(const Window &window);
  void _create_render_pass();
  void _create_command_pool();
  void _create_depth_image();
  void _create_sync_objects();
  // ****************************

  vk::Instance m_instance;
  vk::DebugUtilsMessengerEXT m_debug_messenger;
  vk::SurfaceKHR m_surface;
  vk::PhysicalDevice m_physical_device;
  vk::Device m_device;
  vk::Queue m_graphics_queue;
  vk::Queue m_present_queue;
  std::unique_ptr<SwapChain> m_swap_chain;
  vk::RenderPass m_render_pass;
  vk::CommandPool m_graphic_command_pool;
  std::vector<vk::CommandBuffer> m_graphic_command_buffers;
  vk::Image m_depth_image;
  vk::DeviceMemory m_depth_image_memory;
  vk::ImageView m_depth_image_view;
  std::vector<vk::Semaphore> m_image_available_semaphores;
  std::vector<vk::Semaphore> m_render_finished_semaphores;
  std::vector<vk::Fence> m_in_flight_fences;
  std::vector<vk::Fence> m_images_in_flight;

  size_t m_current_frame;
  std::vector<bool> m_command_buffer_created;
};
} // namespace vulkan
} // namespace core