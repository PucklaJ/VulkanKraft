#pragma once
#include "../window.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class Context {
public:
  Context(const Window &window);
  ~Context();

private:
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
  static bool _has_validation_layer_support(
      const std::vector<const char *> &layer_names) noexcept;
  static void _populate_debug_messenger_create_info(
      vk::DebugUtilsMessengerCreateInfoEXT &di) noexcept;
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  _debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData);

  void _create_instance(const Window &window);

  vk::Instance m_instance;
};
} // namespace vulkan
} // namespace core