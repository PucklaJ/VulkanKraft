#pragma once

#include "core/gui/button.hpp"
#include "core/gui/context.hpp"
#include "core/resource_hodler.hpp"
#include "core/settings.hpp"
#include "core/text/text.hpp"
#include "scene/scene.hpp"

class MainMenuScene : public scene::Scene {
public:
  MainMenuScene(const core::vulkan::Context &context,
                core::ResourceHodler &resource_hodler,
                const core::Settings &settings, core::Window &window,
                const glm::mat4 &projection);

  std::unique_ptr<scene::Scene> update(core::Window &window,
                                       const float delta_time) override;
  void render(const core::vulkan::RenderCall &render_call,
              const float delta_time) override;

private:
  static constexpr int continue_gamepad_button = GLFW_GAMEPAD_BUTTON_CROSS;
  static constexpr int new_world_gamepad_button = GLFW_GAMEPAD_BUTTON_SQUARE;

  core::gui::Context m_gui_context;
  core::gui::Button *m_continue_button;
  core::gui::Button *m_new_world_button;
  core::text::Text m_title_text;

  std::unique_ptr<scene::Scene> m_next_scene;

  const core::vulkan::Context &m_context;
  core::ResourceHodler &m_hodler;
  const core::Settings &m_settings;
  const glm::mat4 &m_projection;
};