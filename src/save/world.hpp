#pragma once

#include "../chunk/block.hpp"
#include <filesystem>
#include <map>
#include <optional>

namespace save {
class World {
public:
  World(const std::filesystem::path &folder);

  std::optional<std::array<uint8_t, chunk::block_width * chunk::block_depth *
                                        chunk::block_height>>
  load_chunk(const std::pair<int, int> &chunk_position) const;
  void store_chunk(
      const std::pair<int, int> &chunk_position,
      const std::array<uint8_t, chunk::block_width * chunk::block_depth *
                                    chunk::block_height> &block_array);

private:
  // Stores all file names of all available chunks in the save folder
  std::map<std::pair<int, int>, std::filesystem::path> m_chunk_file_names;

  const std::filesystem::path m_folder;
};
} // namespace save
