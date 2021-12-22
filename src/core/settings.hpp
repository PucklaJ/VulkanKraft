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
  static constexpr char settings_folder_name[] = ".vulkankraft";
  static constexpr char world_save_folder_name[] = "worlds";
  static constexpr char settings_file_name[] = "settings.json";
  static constexpr float pixel_scale = 2.0f;

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
  inline std::filesystem::path get_controller_db_file_name() const {
    return settings_folder / controller_db_file_name;
  }

private:
  static constexpr char controller_db_file_name[] = "controller_db.txt";
  static constexpr char controller_db_temp_file_name[] =
      "controller_db.txt.tmp";

  static constexpr char msaa_samples_key[] = "msaa";
  static constexpr char max_fps_key[] = "fps";
  static constexpr char window_width_key[] = "width";
  static constexpr char window_height_key[] = "height";
  static constexpr char field_of_view_key[] = "fov";
  static constexpr char render_distance_key[] = "render_distance";

  // Downloads the sdl controller db file from the master branch of
  // https://github.com/gabomdq/SDL_GameControllerDB
  // returns wether the operation was successfull
  bool _download_master_sdl_game_controller_db() const;
  // callback for curl (CURLOPT_WRITEFUNCTION)
  static size_t _download_curl_callback(char *ptr, size_t size, size_t nmemb,
                                        void *userdata);
  // Returns wether the current controller db file is old enough for an update
  bool _should_update_controller_db_file() const;
};
} // namespace core