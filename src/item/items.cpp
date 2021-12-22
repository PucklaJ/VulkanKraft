#include "items.hpp"
#include "../core/exception.hpp"

namespace item {
core::Texture &get_item_texture(const Items item,
                                core::ResourceHodler &hodler) {
  switch (item) {
  case Items::DIRT_BLOCK:
    return hodler.get_texture(
        core::ResourceHodler::dirt_block_item_texture_name);
  case Items::GRASS_BLOCK:
    return hodler.get_texture(
        core::ResourceHodler::grass_block_item_texture_name);
  default:
    throw core::VulkanKraftException("requesting texture for invalid item: " +
                                     std::to_string(static_cast<int>(item)));
  }
}
} // namespace item