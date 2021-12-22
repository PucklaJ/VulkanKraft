#pragma once
#include "../core/resource_hodler.hpp"
#include "../core/texture.hpp"

namespace item {
enum Items { NONE, DIRT_BLOCK, GRASS_BLOCK };

core::Texture &get_item_texture(const Items item, core::ResourceHodler &hodler);
} // namespace item