#pragma once

#include "../chunk/block.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <map>
#include <optional>

namespace save {
class World {
public:
  struct MetaData {
    size_t seed;
  };
  struct PlayerData {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec2 rotation;
  };

  World(const std::filesystem::path &folder);

  std::optional<std::array<uint8_t, chunk::block_width * chunk::block_depth *
                                        chunk::block_height>>
  load_chunk(const std::pair<int, int> &chunk_position) const;
  void store_chunk(
      const std::pair<int, int> &chunk_position,
      const std::array<uint8_t, chunk::block_width * chunk::block_depth *
                                    chunk::block_height> &block_array);
  std::optional<MetaData> read_meta_data() const;
  void write_meta_data(const MetaData &meta_data);
  std::optional<PlayerData> read_player_data() const;
  void write_player_data(const PlayerData &player_data);

private:
  // Stores all file names of all available chunks in the save folder
  std::map<std::pair<int, int>, std::filesystem::path> m_chunk_file_names;
  // The file name of the file storing meta data about the world
  std::filesystem::path m_meta_data_file_name;
  // The file name of the file storing all player data about the world
  std::filesystem::path m_player_data_file_name;

  const std::filesystem::path m_folder;
};
} // namespace save
