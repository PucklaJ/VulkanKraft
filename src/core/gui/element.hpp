#pragma once
#include "../vulkan/context.hpp"
#include "../window.hpp"

namespace core {
namespace gui {
class Context;

class Element {
public:
  Element(Context *gui_context) : m_gui_context(gui_context) {}
  virtual ~Element() = default;

  virtual void update(Window &window) = 0;
  virtual void render(const vulkan::Context &vulkan_context,
                      const vulkan::RenderCall &render_call) = 0;

protected:
  static bool _mouse_collides(const Window &window, const glm::vec2 &position,
                              const glm::uvec2 &size);

  Context *m_gui_context;
};
} // namespace gui
} // namespace core
