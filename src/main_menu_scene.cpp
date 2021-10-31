#include "main_menu_scene.hpp"
#include "ingame_scene.hpp"

MainMenuScene::MainMenuScene(const core::vulkan::Context &context,
                             core::ResourceHodler &resource_hodler,
                             const core::Settings &settings,
                             core::Window &window, const glm::mat4 &projection)
    : m_context(context), m_hodler(resource_hodler), m_settings(settings),
      m_gui_context(
          context, window,
          resource_hodler.get_font(core::ResourceHodler::debug_font_name)),
      m_title_text(
          context,
          resource_hodler.get_font(core::ResourceHodler::debug_font_name),
          L"VulkanKraft"),
      m_projection(projection) {

  glm::vec2 current_pos(settings.window_width / 2,
                        settings.window_height / 2 - 50 - 15);
  m_continue_button = m_gui_context.add_element<core::gui::Button>(
      &m_gui_context, std::ref(resource_hodler), L"Continue", 250, 100,
      current_pos);
  current_pos.y += 50 + 30 + 50;
  m_new_world_button = m_gui_context.add_element<core::gui::Button>(
      &m_gui_context, std::ref(resource_hodler), L"New World", 250, 100,
      current_pos);

  m_title_text.set_font_size(100.0f);
  current_pos.y -= 50 + 30 + 100 + 50 + m_title_text.get_height();
  current_pos.x -= m_title_text.get_width() / 2;
  m_title_text.set_position(current_pos);

  m_continue_button->on_click = [&]() {
    m_next_scene = std::make_unique<InGameScene>(m_context, m_hodler, window,
                                                 m_settings, m_projection);
  };

  m_new_world_button->on_click = [&]() {
    m_next_scene = std::make_unique<InGameScene>(
        m_context, m_hodler, window, m_settings, m_projection, true,
        static_cast<size_t>(time(nullptr)));
  };
}

std::unique_ptr<scene::Scene> MainMenuScene::update(core::Window &window,
                                                    const float delta_time) {
  m_gui_context.update();

  if (window.is_gamepad_connected()) {
    if (window.gamepad_button_just_pressed(continue_gamepad_button)) {
      m_continue_button->on_click();
    } else if (window.gamepad_button_just_pressed(new_world_gamepad_button)) {
      m_new_world_button->on_click();
    }
  }

  return std::move(m_next_scene);
}

void MainMenuScene::render(const core::vulkan::RenderCall &render_call,
                           const float delta_time) {
  m_gui_context.render(render_call);

  core::text::Text::bind_shader(render_call);
  m_title_text.render(render_call);
}
