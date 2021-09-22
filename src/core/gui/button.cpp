#include "button.hpp"
#include "context.hpp"

namespace core {
namespace gui {

Button::Button(Context *gui_context, const ResourceHodler &resource_hodler,
               const std::wstring &label_text, const uint32_t width,
               const uint32_t height, const glm::vec2 &position)
    : Element(gui_context), m_position(position), m_size(width, height),
      m_text(gui_context->m_vulkan_context, gui_context->m_text_shader,
             gui_context->m_text_font, label_text),
      m_grey_texture(
          _create_grey_rectangle(gui_context->m_vulkan_context, 10, 10)),
      m_greyer_texture(
          _create_greyer_texture(gui_context->m_vulkan_context, 10, 10)),
      m_black_texture(
          _create_black_texture(gui_context->m_vulkan_context, 10, 10)) {
  m_render2d.set_texture(&m_grey_texture);
  m_render2d.set_model_matrix(
      position,
      glm::vec2(static_cast<float>(width), static_cast<float>(height)) / 10.0f);
  const auto text_width{m_text.get_width()};
  const auto text_height{m_text.get_height()};
  m_text.set_position(position - glm::vec2(static_cast<float>(text_width),
                                           static_cast<float>(text_height)) /
                                     2.0f);
}

void Button::update(Window &window) {
  if (_mouse_collides(window, m_position, m_size)) {
    m_render2d.set_texture(&m_greyer_texture);

    if (window.get_mouse().button_is_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
      m_render2d.set_texture(&m_black_texture);
    } else if (on_click &&
               window.get_mouse().button_was_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
      on_click();
    }
  } else {
    m_render2d.set_texture(&m_grey_texture);
  }
}

void Button::render(const vulkan::Context &vulkan_context,
                    const vulkan::RenderCall &render_call) {
  Render2D::bind_shader(render_call);
  m_render2d.render(render_call);

  m_gui_context->m_text_shader.bind(render_call);
  m_text.render(render_call);
}

Texture Button::_create_grey_rectangle(const vulkan::Context &context,
                                       const uint32_t width,
                                       const uint32_t height) {
  std::vector<uint8_t> data(width * height * 4);
  memset(data.data(), 128, data.size());

  for (uint32_t x = 0; x < width; x++) {
    for (uint32_t y = 0; y < height; y++) {
      data[x * 4 + y * width * 4 + 3] = 255;
    }
  }

  return Texture::Builder()
      .dimensions(width, height)
      .filter(vk::Filter::eNearest)
      .build(context, data.data());
}

Texture Button::_create_greyer_texture(const vulkan::Context &context,
                                       const uint32_t width,
                                       const uint32_t height) {
  std::vector<uint8_t> data(width * height * 4);
  memset(data.data(), 64, data.size());

  for (uint32_t x = 0; x < width; x++) {
    for (uint32_t y = 0; y < height; y++) {
      data[x * 4 + y * width * 4 + 3] = 255;
    }
  }

  return Texture::Builder()
      .dimensions(width, height)
      .filter(vk::Filter::eNearest)
      .build(context, data.data());
}

Texture Button::_create_black_texture(const vulkan::Context &context,
                                      const uint32_t width,
                                      const uint32_t height) {
  std::vector<uint8_t> data(width * height * 4);
  memset(data.data(), 0, data.size());

  for (uint32_t x = 0; x < width; x++) {
    for (uint32_t y = 0; y < height; y++) {
      data[x * 4 + y * width * 4 + 3] = 255;
    }
  }

  return Texture::Builder()
      .dimensions(width, height)
      .filter(vk::Filter::eNearest)
      .build(context, data.data());
}

} // namespace gui
} // namespace core
