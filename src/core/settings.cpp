#include "settings.hpp"
#include "exception.hpp"
#include "log.hpp"
#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace core {
Settings::Settings()
    : msaa_samples(vk::SampleCountFlagBits::e4), max_fps(60),
      window_width(1280), window_height(720),
      field_of_view(glm::radians(70.0f)), render_distance(6) {
  // handle settings folder
#ifdef _WIN32
  const char *appdata = getenv("APPDATA");
  settings_folder = std::filesystem::path(appdata) / settings_folder_name;
#else
  const char *home_dir = getenv("HOME");
  settings_folder = std::filesystem::path(home_dir) / settings_folder_name;
#endif

  std::filesystem::create_directories(settings_folder);

  // load settings file
  const auto settings_file(settings_folder / settings_file_name);
  if (!std::filesystem::exists(settings_file)) {
    write_settings_file();
  } else {
    std::ifstream file;
    file.open(settings_file);
    if (file.fail()) {
      Log::warning("failed to open settings file");
    } else {
      try {
        json json_file;
        file >> json_file;
        file.close();

        if (!json_file.is_object()) {
          throw VulkanKraftException("the settings file is not an object");
        }

        if (json_file.contains(msaa_samples_key)) {
          msaa_samples =
              json_file[msaa_samples_key].get<decltype(msaa_samples)>();
        }
        if (json_file.contains(max_fps_key)) {
          max_fps = json_file[max_fps_key].get<decltype(max_fps)>();
        }
        if (json_file.contains(window_width_key)) {
          window_width =
              json_file[window_width_key].get<decltype(window_width)>();
        }
        if (json_file.contains(window_height_key)) {
          window_height =
              json_file[window_height_key].get<decltype(window_height)>();
        }
        if (json_file.contains(field_of_view_key)) {
          field_of_view = glm::radians(
              json_file[field_of_view_key].get<decltype(field_of_view)>());
        }
        if (json_file.contains(render_distance_key)) {
          render_distance =
              json_file[render_distance_key].get<decltype(render_distance)>();
        }

        Log::info("Successfully read " + settings_file.string());
      } catch (const json::exception &e) {
        Log::warning(std::string("failed to read settings file: ") + e.what());
      } catch (const VulkanKraftException &e) {
        Log::warning(e.what());
      }
    }
  }
}

Settings::~Settings() { write_settings_file(); }

void Settings::write_settings_file() const {
  const auto settings_file(settings_folder / settings_file_name);

  std::ofstream file;
  file.open(settings_file);
  if (file.fail()) {
    Log::warning("failed to open settings file for writing");
    return;
  }

  try {
    file << json({{msaa_samples_key, msaa_samples},
                  {max_fps_key, max_fps},
                  {window_width_key, window_width},
                  {window_height_key, window_height},
                  {field_of_view_key, glm::degrees(field_of_view)},
                  {render_distance_key, render_distance}});

    Log::info("Successfully wrote to " + settings_file.string());
  } catch (const json::exception &e) {
    Log::warning(std::string("failed to write settings file: ") + e.what());
  }
}

} // namespace core