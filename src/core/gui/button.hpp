#pragma once
#include "../render_2d.hpp"
#include "../resource_hodler.hpp"
#include "../text/text.hpp"
#include "element.hpp"

namespace core {
namespace gui {

class Button : public Element {
public:
  Button(Context *gui_context, const ResourceHodler &resource_hodler,
         const std::wstring &label_text, const uint32_t width,
         const uint32_t height, const glm::vec2 &position);

  void update(Window &window) override;
  void render(const vulkan::Context &vulkan_context,
              const vulkan::RenderCall &render_call) override;

private:
  static Texture _create_grey_rectangle(const vulkan::Context &context,
                                        const uint32_t width,
                                        const uint32_t height);

  Render2D m_render2d;
  text::Text m_text;

  Texture m_grey_texture;
};

} // namespace gui
} // namespace core
