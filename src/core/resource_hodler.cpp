#include "resource_hodler.hpp"

namespace core {
ResourceHodler::ResourceHodler(const core::vulkan::Context &context)
    : m_context(context) {}

void ResourceHodler::hodl_texture(std::filesystem::path file_path,
                                  Texture texture) {
  m_hodled_textures.emplace(std::move(file_path), std::move(texture));
}
} // namespace core