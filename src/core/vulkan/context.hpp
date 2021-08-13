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

// This class initialises Vulkan and holds all objects necessary for using
// vulkan
class Context {
public:
  // Stores indices for the graphics and present queue family
  class QueueFamilyIndices {
  public:
    // Retrieves the indices from the physical device and surface
    QueueFamilyIndices(const vk::PhysicalDevice &device,
                       const vk::SurfaceKHR &surface);

    inline bool is_complete() const {
      return graphics_family.has_value() && present_family.has_value();
    }

    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
  };

  // This class holds information about the support of the swap chain
  class SwapChainSupportDetails {
  public:
    // Retrieve all details from the physical device and surface
    SwapChainSupportDetails(const vk::PhysicalDevice &device,
                            const vk::SurfaceKHR &surface);

    vk::SurfaceCapabilitiesKHR capabilities;
    // All supported formats
    std::vector<vk::SurfaceFormatKHR> formats;
    // All supported present modes
    std::vector<vk::PresentModeKHR> present_modes;
  };

  // Stores information about what the physical device supports
  class PhysicalDeviceInfo {
  public:
    // Retrieve all the information from the physical device and surface
    PhysicalDeviceInfo(const vk::PhysicalDevice &device,
                       const vk::SurfaceKHR &surface);

    const vk::PhysicalDeviceProperties properties;
    // The format choosen for depth textures and attachments
    const vk::Format depth_format;
    const QueueFamilyIndices queue_family_indices;
    const vk::SampleCountFlagBits max_msaa_samples;
    SwapChainSupportDetails swap_chain_support_details;
    // Wether the physical device supports linear blitting (required for the
    // generation of the mip maps)
    bool linear_blitting_support;

  private:
    // Returns the correct format for the given tiling and features
    static vk::Format
    _find_supported_format(const vk::PhysicalDevice &device,
                           const std::vector<vk::Format> &candidates,
                           vk::ImageTiling tiling,
                           vk::FormatFeatureFlags features);
    // Returns the supported depth format of the physical device
    static inline vk::Format
    _find_depth_format(const vk::PhysicalDevice &device) {
      return _find_supported_format(
          device,
          {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
           vk::Format::eD24UnormS8Uint},
          vk::ImageTiling::eOptimal,
          vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
    // Returns the maximum supported sample count for multisampling
    static vk::SampleCountFlagBits
    _get_max_usable_sample_count(const vk::PhysicalDeviceProperties &props);
  };

  friend class SwapChain;
  friend class RenderCall;

  // Functions that need to be loaded manually
  static PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
  static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

  Context(Window &window, Settings &settings);
  ~Context();

  // ***** inline methods *****
  // A nice quick way to retrieve the number of images in the swap chain
  inline size_t get_swap_chain_image_count() const {
    return m_swap_chain->get_image_count();
  }
  // A nice quick way to retrieve the render pass of the swap chain
  inline const vk::RenderPass &get_swap_chain_render_pass() const {
    return m_swap_chain->get_render_pass();
  }
  inline const vk::Device &get_device() const noexcept { return m_device; }
  inline const PhysicalDeviceInfo &get_physical_device_info() const noexcept {
    return *m_physical_device_info;
  }
  // Returns a command buffer used for graphics commands that will be executed
  // immediately
  inline vk::CommandBuffer begin_single_time_graphics_commands() const {
    return _begin_single_time_commands(m_device, m_graphic_command_pool);
  }
  // Executes all commands of buffer
  inline void
  end_single_time_graphics_commands(vk::CommandBuffer buffer) const {
    _end_single_time_commands(m_device, m_graphic_command_pool,
                              m_graphics_queue, std::move(buffer));
  }
  // ***************************

  // **** utility methods *******
  // Transistion the layout of image from old_layout to new_layout
  void transition_image_layout(const vk::Image &image, vk::Format format,
                               vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               uint32_t mip_levels) const;
  // Returns the correct memory type required for the props
  uint32_t find_memory_type(uint32_t type_filter,
                            vk::MemoryPropertyFlags props) const;
  // ****************************

  // ***** render methods ********
  // Begins a frame and returns a RenderCall if a swap chain image could be
  // retrieved successfully
  std::optional<RenderCall> render_begin();
  // *****************************

private:
  // ****** constants **************
  // Some values used to initialise vulkan
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
  // The callback used for the validation layers
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  _debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData);
  // *********************************

  // ****** Utilities ****************
  // Returns wether the given physical device supports all required features and
  // extensions
  static bool
  _is_device_suitable(const vk::PhysicalDevice &device,
                      const vk::SurfaceKHR &surface,
                      const std::vector<const char *> &extension_names);
  // Returns wether the given physical device supports the given extensions
  static bool _device_has_extension_support(
      const vk::PhysicalDevice &device,
      const std::vector<const char *> &extension_names);
  // Returns wether the context supports the given validation layers
  static bool _has_validation_layer_support(
      const std::vector<const char *> &layer_names) noexcept;

  // Returns a command buffer used to execute commands immediately
  static vk::CommandBuffer
  _begin_single_time_commands(const vk::Device &device,
                              const vk::CommandPool &command_pool);
  // Execute all commands of the given command buffer created from the given
  // command pool
  static void _end_single_time_commands(const vk::Device &device,
                                        const vk::CommandPool &command_pool,
                                        const vk::Queue &queue,
                                        vk::CommandBuffer command_buffer);
  // Returns wether the given format contains a stencil component
  static inline constexpr bool _has_stencil_component(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint ||
           format == vk::Format::eD24UnormS8Uint;
  }
  // ****************************

  // ****** Initialisation ******
  // Creates the vulkan instance
  void _create_instance(const Window &window,
                        const std::vector<const char *> &validation_layers);
  // Creates the debug messenger and connects it with the debug callback
  void _setup_debug_messenger();
  // Create the surface for the window
  void _create_surface(const Window &window);
  // Pick a suitable physical device
  void _pick_physical_device(const std::vector<const char *> &extensions);
  // Create the logical device from the physical device
  void
  _create_logical_device(const std::vector<const char *> &extensions,
                         const std::vector<const char *> &validation_layers);
  // Create the command pool for all command buffers
  void _create_command_pool();
  // Create the swap chain for the window
  void _create_swap_chain(const Window &window);
  // Allocate all command buffers for the different swap chain images
  void _allocate_command_buffers();
  // Create semaphores and fences for syncing
  void _create_sync_objects();
  // ****************************

  // A callback used to handle the resize of the framebuffer (or window)
  void _handle_framebuffer_resize();

  vk::Instance m_instance;
  vk::DebugUtilsMessengerEXT m_debug_messenger;
  vk::SurfaceKHR m_surface;
  vk::PhysicalDevice m_physical_device;
  vk::Device m_device;
  // Queue used for graphics commands
  vk::Queue m_graphics_queue;
  // Queue used to execute commands for presenting to the surface
  vk::Queue m_present_queue;
  std::unique_ptr<SwapChain> m_swap_chain;
  // Command pool for all graphics command buffers
  vk::CommandPool m_graphic_command_pool;
  // One command buffer for every swap chain image
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