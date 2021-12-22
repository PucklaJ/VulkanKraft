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
          core::Settings::pixel_scale) {}

void Inventory::update(core::Window &window) {
  const auto [width, height] = window.get_framebuffer_size();
  m_hotbar.set_model_matrix(
      glm::vec2(width / 2, height - m_hotbar_height / 2 - 50),
      glm::vec2(core::Settings::pixel_scale, core::Settings::pixel_scale));
}

void Inventory::render(const core::vulkan::RenderCall &render_call) {
  core::Render2D::bind_shader(render_call);
  m_hotbar.render(render_call);
}

} // namespace item