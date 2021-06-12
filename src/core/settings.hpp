#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
class Settings {
public:
  static constexpr char window_title[] = "VulkanKraft";
  static constexpr float near_plane = 0.01f;
  static constexpr float far_plane = 1000.0f;

  Settings();

  vk::SampleCountFlagBits msaa_samples;
  size_t max_fps;
  uint32_t window_width;
  uint32_t window_height;
  float field_of_view;
  int render_distance;
};
} // namespace core