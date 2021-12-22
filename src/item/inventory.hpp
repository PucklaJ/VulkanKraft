#pragma once
#include "../core/render_2d.hpp"
#include "../core/resource_hodler.hpp"
#include "../core/vulkan/render_call.hpp"
#include "../core/window.hpp"
#include "items.hpp"
#include <array>
#include <glm/glm.hpp>

namespace item {
// Holds items. 3 rows of 9 and the hotbar
class Inventory : public std::array<Items, 9 * 4> {
public:
  Inventory(core::ResourceHodler &hodler);

  void update(core::Window &window);
  void render(const core::vulkan::RenderCall &render_call);

private:
  core::Render2D m_hotbar;
  core::Render2D m_inventory_screen;
  core::Render2D m_item_render;

  glm::vec2 m_hotbar_position;

  const uint32_t m_hotbar_height;
  const uint32_t m_hotbar_width;
  core::ResourceHodler &m_hodler;
};

} // namespace item
