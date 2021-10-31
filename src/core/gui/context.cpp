#include "context.hpp"

namespace core {
namespace gui {

Context::Context(const vulkan::Context &vulkan_context, Window &window,
                 text::Font &text_font)
    : m_vulkan_context(vulkan_context), m_window(window),
      m_text_font(text_font) {}

void Context::update() {
  for (const auto &e : m_elements) {
    e->update(m_window);
  }
}

void Context::render(const vulkan::RenderCall &render_call) {
  for (const auto &e : m_elements) {
    e->render(m_vulkan_context, render_call);
  }
}

} // namespace gui
} // namespace core
