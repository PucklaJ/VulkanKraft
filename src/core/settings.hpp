#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
class Settings {
public:
  static constexpr char window_title[] = "VulkanKraft";

  Settings();

  vk::SampleCountFlagBits msaa_samples;
  size_t max_fps;
  uint32_t window_width;
  uint32_t window_height;
};
} // namespace core