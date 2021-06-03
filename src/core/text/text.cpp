#include "text.hpp"
#include <iostream>

namespace core {
namespace text {
Shader Text::build_shader(const vulkan::Context &context,
                          const Settings &settings) {
  GlobalUniform global;
  global.proj = glm::mat4(1.0f);

  auto shader(Shader::Builder()
                  .vertex_attribute<glm::vec2>()
                  .vertex_attribute<glm::vec2>()
                  .vertex("shaders_spv/text.vert.spv")
                  .fragment("shaders_spv/text.frag.spv")
                  .uniform_buffer(vk::ShaderStageFlagBits::eVertex, global)
                  .texture()
                  .build(context, settings));

  return shader;
}

Text::Text(const vulkan::Context &context, Font &font,
           const std::wstring &string)
    : m_context(context), m_font(font), m_string(string), m_font_size(50.0f),
      m_text_texture(_build_texture()),
      m_buffer(context, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(Mesh)) {
  const float w{m_texture_width};
  const float h{m_texture_height};

  Mesh mesh;
  mesh[0] = Vertex(0.0f, h, 0.0f, 1.0f);
  mesh[1] = Vertex(w, h, 1.0f, 1.0f);
  mesh[2] = Vertex(w, 0.0f, 1.0f, 0.0f);
  mesh[3] = Vertex(w, 0.0f, 1.0f, 0.0f);
  mesh[4] = Vertex(0.0f, 0.0f, 0.0f, 0.0f);
  mesh[5] = Vertex(0.0f, h, 0.0f, 1.0f);

  m_buffer.set_data(&mesh, sizeof(mesh));
}

void Text::set_string(const std::wstring &string) {
  m_string = string;
  m_text_texture = _build_texture();
}

void Text::set_font_size(const float font_size) {
  m_font_size = font_size;
  m_text_texture = _build_texture();
}

void Text::render(const vulkan::RenderCall &render_call) {
  m_buffer.bind(render_call);
  render_call.render_vertices(6);
}

Texture Text::_build_texture() {
  size_t pixel_width, pixel_height;
  const auto pixels(
      m_font.create_bitmap(m_string, m_font_size, pixel_width, pixel_height));
  m_texture_width = pixel_width;
  m_texture_height = pixel_height;
  return Texture::Builder()
      .dimensions(pixel_width, pixel_height)
      .format(vk::Format::eR32Sfloat)
      .build(m_context, pixels.data());
}
} // namespace text
} // namespace core
