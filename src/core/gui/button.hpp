#pragma once
#include "../render_2d.hpp"
#include "../resource_hodler.hpp"
#include "../text/text.hpp"
#include "element.hpp"
#include <functional>

namespace core {
namespace gui {

class Button : public Element {
public:
  using click_callback = std::function<void()>;

  Button(Context *gui_context, const ResourceHodler &resource_hodler,
         const std::wstring &label_text, const uint32_t width,
         const uint32_t height, const glm::vec2 &position = glm::vec2());

  void update(Window &window) override;
  void render(const vulkan::Context &vulkan_context,
              const vulkan::RenderCall &render_call) override;
  void set_position(const glm::vec2 &position);

  click_callback on_click;

private:
  static Texture _create_grey_rectangle(const vulkan::Context &context,
                                        const uint32_t width,
                                        const uint32_t height);
  static Texture _create_greyer_texture(const vulkan::Context &context,
                                        const uint32_t width,
                                        const uint32_t height);
  static Texture _create_black_texture(const vulkan::Context &context,
                                       const uint32_t width,
                                       const uint32_t height);

  glm::vec2 m_position;
  const glm::uvec2 m_size;
  Render2D m_render2d;
  text::Text m_text;

  Texture m_grey_texture;
  Texture m_greyer_texture;
  Texture m_black_texture;
};

} // namespace gui
} // namespace core
