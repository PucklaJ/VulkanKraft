#pragma once
#include "texture.hpp"
#include <filesystem>
#include <map>

namespace core {
class ResourceHodler {
public:
  ResourceHodler(const vulkan::Context &context);

  void hodl_texture(std::filesystem::path file_path, Texture texture);

private:
  std::map<std::filesystem::path, Texture> m_hodled_textures;

  const vulkan::Context &m_context;
};
} // namespace core