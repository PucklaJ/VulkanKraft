#include "inventory.hpp"
#include "../core/settings.hpp"

namespace item {
Inventory::Inventory(core::ResourceHodler &hodler)
    : m_hotbar(&hodler.get_texture(
          core::ResourceHodler::inventory_hotbar_texture_name)),
      m_hotbar_height(
          hodler
              .get_texture(core::ResourceHodler::inventory_hotbar_texture_name)
              .get_height() *
          core::Settings::pixel_scale),
      m_hotbar_width(
          hodler
              .get_texture(core::ResourceHodler::inventory_hotbar_texture_name)
              .get_width() *
          core::Settings::pixel_scale),
      m_hodler(hodler) {
  std::fill(begin(), end(), Items::NONE);

  (*this)[0] = Items::DIRT_BLOCK;
  (*this)[1] = Items::GRASS_BLOCK;
  (*this)[8] = Items::GRASS_BLOCK;
}

void Inventory::update(core::Window &window) {
  const auto [width, height] = window.get_framebuffer_size();
  m_hotbar_position.x = width / 2;
  m_hotbar_position.y = height - m_hotbar_height / 2 - 50;
  m_hotbar.set_model_matrix(
      m_hotbar_position,
      glm::vec2(core::Settings::pixel_scale, core::Settings::pixel_scale));
}

void Inventory::render(const core::vulkan::RenderCall &render_call) {
  core::Render2D::bind_shader(render_call);

  // Render all items
  for (size_t i = 0; i < 9; i++) {
    if ((*this)[i] != Items::NONE) {
      m_item_render.set_texture(&get_item_texture((*this)[i], m_hodler));
      m_item_render.set_model_matrix(
          m_hotbar_position - glm::vec2(m_hotbar_width / 2, 0.0f) +
              glm::vec2((i * (32.0f + 2.0f * 2.0f) + (2.0f + 32.0f / 2.0f)) *
                            core::Settings::pixel_scale,
                        0.0f),
          glm::vec2(core::Settings::pixel_scale, core::Settings::pixel_scale));
      m_item_render.render(render_call);
    }
  }

  m_hotbar.render(render_call);
}

} // namespace item