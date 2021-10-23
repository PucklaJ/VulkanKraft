#include "settings.hpp"
#include "exception.hpp"
#include "log.hpp"
#include <cstdlib>
#include <curl/curl.h>
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

  // handle sdl controller db file
  if (_download_master_sdl_game_controller_db()) {
    // overwrite normal controller db file
    const auto controller_db_file_path(get_controller_db_file_name());

    std::ofstream db_file;
    db_file.open(controller_db_file_path);
    if (db_file.fail()) {
      Log::warning("failed to open controller db file for writing");
    } else {
      std::ifstream temp_db_file;
      temp_db_file.open(settings_folder / controller_db_temp_file_name,
                        std::ios_base::ate);
      if (temp_db_file.fail()) {
        Log::warning("failed to open controller db temp file for reading");
      } else {
        const auto file_size{temp_db_file.tellg()};
        temp_db_file.seekg(0);
        std::vector<char> buffer;
        buffer.resize(file_size);
        temp_db_file.read(buffer.data(), file_size);
        temp_db_file.close();

        db_file.write(buffer.data(), file_size);
      }
    }
  }
  std::filesystem::remove(settings_folder / controller_db_temp_file_name);
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

bool Settings::_download_master_sdl_game_controller_db() const {
  constexpr char url[] = "https://raw.githubusercontent.com/gabomdq/"
                         "SDL_GameControllerDB/master/gamecontrollerdb.txt";
  const auto db_file_name(settings_folder / controller_db_temp_file_name);

  curl_global_init(CURL_GLOBAL_ALL);

  try {
    if (auto *curl = curl_easy_init(); curl) {
      std::ofstream db_file;

      curl_easy_setopt(curl, CURLOPT_URL, url);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _download_curl_callback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                       reinterpret_cast<void *>(&db_file));

      db_file.open(db_file_name);
      if (db_file.fail()) {
        curl_easy_cleanup(curl);
        throw VulkanKraftException(
            "failed to open controller db temp file for writing");
      }

      const auto res{curl_easy_perform(curl)};
      if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw VulkanKraftException(std::string("failed to perform curl: ") +
                                   curl_easy_strerror(res));
      }

      curl_easy_cleanup(curl);
    } else {
      throw VulkanKraftException("failed to initialise curl");
    }
  } catch (const VulkanKraftException &e) {
    Log::warning(
        std::string("failed to download master sdl controller db file: ") +
        e.what());
    curl_global_cleanup();
    return false;
  }

  curl_global_cleanup();

  Log::info("Successfully downloaded master sdl controller db file to " +
            db_file_name.string());

  return true;
}

size_t Settings::_download_curl_callback(char *ptr, size_t size, size_t nmemb,
                                         void *userdata) {
  auto *db_file = reinterpret_cast<std::ofstream *>(userdata);
  db_file->write(ptr, nmemb);
  return nmemb;
}

} // namespace core