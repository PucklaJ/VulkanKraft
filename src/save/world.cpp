#include "world.hpp"
#include "../core/exception.hpp"
#include <fstream>
#include <sstream>

namespace save {

World::World(const std::filesystem::path &folder) : m_folder(folder) {
  std::filesystem::create_directories(folder);

  std::stringstream converter;

  // Store all chunk file names in the map
  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (!entry.is_directory()) {
      const auto &file_name(entry.path());

      if (file_name.extension() == ".chunk") {
        const auto file_name_str(file_name.stem().string());
        const auto space_pos{file_name_str.find('_')};

        const auto x_str(file_name_str.substr(0, space_pos));
        const auto y_str(file_name_str.substr(space_pos + 1));

        std::pair<int, int> chunk_pos;

        converter.clear();
        converter << x_str;
        converter >> chunk_pos.first;
        converter.clear();
        converter << y_str;
        converter >> chunk_pos.second;

        m_chunk_file_names.emplace(std::move(chunk_pos), file_name);
      } else if (file_name.filename() == "meta_data") {
        m_meta_data_file_name = file_name;
      }
    }
  }
}

std::optional<std::array<uint8_t, chunk::block_width * chunk::block_depth *
                                      chunk::block_height>>
World::load_chunk(const std::pair<int, int> &chunk_position) const {
  if (m_chunk_file_names.find(chunk_position) == m_chunk_file_names.end()) {
    return std::nullopt;
  }

  const auto &file_name(m_chunk_file_names.at(chunk_position));
  std::ifstream file;
  file.open(file_name, std::ios_base::binary);
  if (file.fail()) {
    throw core::VulkanKraftException("failed to load chunk file " +
                                     file_name.string());
  }

  std::array<uint8_t,
             chunk::block_width * chunk::block_depth * chunk::block_height>
      block_array;

  file.read(reinterpret_cast<char *>(block_array.data()), sizeof(block_array));

  return std::make_optional(std::move(block_array));
}

void World::store_chunk(
    const std::pair<int, int> &chunk_position,
    const std::array<uint8_t, chunk::block_width * chunk::block_depth *
                                  chunk::block_height> &block_array) {
  std::filesystem::path file_name;
  if (m_chunk_file_names.find(chunk_position) != m_chunk_file_names.end()) {
    file_name = m_chunk_file_names.at(chunk_position);
  } else {
    std::stringstream stream;
    stream << chunk_position.first << "_" << chunk_position.second << ".chunk";
    file_name = stream.str();
    file_name = m_folder / file_name;
    m_chunk_file_names.emplace(chunk_position, file_name);
  }

  std::ofstream file;
  file.open(file_name, std::ios_base::binary);
  if (file.fail()) {
    throw core::VulkanKraftException("failed to store chunk file " +
                                     file_name.string());
  }

  file.write(reinterpret_cast<const char *>(block_array.data()),
             sizeof(block_array));
}

std::optional<World::MetaData> World::read_meta_data() const {
  if (m_meta_data_file_name.empty()) {
    return std::nullopt;
  }

  std::ifstream file;
  file.open(m_meta_data_file_name, std::ios_base::binary);
  if (file.fail()) {
    throw core::VulkanKraftException(
        "failed to read meta data file of world save");
  }

  MetaData meta_data;

  file.read(reinterpret_cast<char *>(&meta_data), sizeof(meta_data));

  return meta_data;
}

void World::write_meta_data(const World::MetaData &meta_data) {
  if (m_meta_data_file_name.empty()) {
    m_meta_data_file_name = m_folder / "meta_data";
  }

  std::ofstream file;
  file.open(m_meta_data_file_name, std::ios_base::binary);
  if (file.fail()) {
    throw core::VulkanKraftException(
        "failed to write meta data file of world save");
  }

  file.write(reinterpret_cast<const char *>(&meta_data), sizeof(meta_data));
}

} // namespace save
