#pragma once
#include <filesystem>
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
  ~Settings();

  std::filesystem::path settings_folder;

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

  void write_settings_file() const;

private:
  static constexpr char settings_folder_name[] = ".vulkankraft";
  static constexpr char settings_file_name[] = "settings.json";

  static constexpr char msaa_samples_key[] = "msaa";
  static constexpr char max_fps_key[] = "fps";
  static constexpr char window_width_key[] = "width";
  static constexpr char window_height_key[] = "height";
  static constexpr char field_of_view_key[] = "fov";
  static constexpr char render_distance_key[] = "render_distance";
};
} // namespace core