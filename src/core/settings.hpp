#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
// This class holds different settings of the game and will also used to store
// the settings that will be changed via an options menu
class Settings {
public:
  static constexpr char window_title[] = "VulkanKraft";
  // The near plane of the perspective projection matrix used for all 3D objects
  static constexpr float near_plane = 0.01f;
  // The far plane of the perspective projection matrix used for all 3D objects
  static constexpr float far_plane = 1000.0f;

  Settings();

  // Multisampling Anti Aliasing Samples
  vk::SampleCountFlagBits msaa_samples;
  // Maximum Frames per Second used to lock the frame time
  size_t max_fps;
  // Horizontal resolution of the window on start up
  uint32_t window_width;
  // Vertical resolution of the window on start up
  uint32_t window_height;
  // The field of view of the perspective projection matrix used for all 3D
  // objects
  float field_of_view;
  // Defines how far the player will be able to see in chunks
  int render_distance;
};
} // namespace core