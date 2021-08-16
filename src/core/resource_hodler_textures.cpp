#include "resource_hodler.hpp"

namespace textures {
#include <textures/block.hpp>
#include <textures/crosshair.hpp>
} // namespace textures

namespace core {

void ResourceHodler::_load_all_textures(const vulkan::Context &context,
                                        const Settings &settings) {
  // Textures
  load_texture(context, chunk_mesh_texture_name, textures::block_png,
               [](auto &b) { b.filter(vk::Filter::eNearest).mip_maps(); });
  load_texture(context, crosshair_texture_name, textures::crosshair_png,
               [](auto &b) { b.filter(vk::Filter::eNearest); });
}

} // namespace core