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
                  .dynamic_texture(max_texts)
                  .alpha_blending()
                  .build(context, settings));

  return shader;
}

Text::Text(const vulkan::Context &context, Shader &shader, Font &font,
           const std::wstring &string, const glm::vec2 &position,
           const float font_size)
    : m_context(context), m_shader(shader), m_font(font), m_string(string),
      m_font_size(font_size), m_position(position),
      m_text_texture(_build_texture()) {
  m_buffers.reserve(m_context.get_swap_chain_image_count());
  for (size_t i = 0; i < m_context.get_swap_chain_image_count(); i++) {
    m_buffers.emplace_back(m_context, vk::BufferUsageFlagBits::eVertexBuffer,
                           sizeof(Mesh));
  }
  _build_buffers();
}

void Text::set_string(const std::wstring &string) {
  if (string == m_string)
    return;

  m_string = string;
  auto dynamic_sets(m_text_texture.extract_dynamic_sets());
  m_text_texture = _build_texture();
  m_text_texture.set_dynamic_sets(std::move(dynamic_sets));
  _build_buffers();
}

void Text::set_font_size(const float font_size) {
  if (font_size == m_font_size)
    return;

  m_font_size = font_size;
  auto dynamic_sets(m_text_texture.extract_dynamic_sets());
  m_text_texture = _build_texture();
  m_text_texture.set_dynamic_sets(std::move(dynamic_sets));
  _build_buffers();
}

void Text::set_position(const glm::vec2 &position) {
  m_position = position;
  _build_buffers();
}

void Text::render(const vulkan::RenderCall &render_call) {
  if (m_buffer_write_to_perform.image_indices.count(
          render_call.get_swap_chain_image_index()) != 0) {
    m_buffers[render_call.get_swap_chain_image_index()].set_data(
        &m_buffer_write_to_perform.mesh, sizeof(Mesh));
    m_buffer_write_to_perform.image_indices.erase(
        render_call.get_swap_chain_image_index());
  }

  m_shader.bind_dynamic_texture(render_call, m_text_texture);
  m_buffers[render_call.get_swap_chain_image_index()].bind(render_call);
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

void Text::_build_buffers() {
  const float w{static_cast<float>(m_texture_width)};
  const float h{static_cast<float>(m_texture_height)};

  m_buffer_write_to_perform.mesh[0] = Vertex(m_position, 0.0f, h, 0.0f, 1.0f);
  m_buffer_write_to_perform.mesh[1] = Vertex(m_position, w, h, 1.0f, 1.0f);
  m_buffer_write_to_perform.mesh[2] = Vertex(m_position, w, 0.0f, 1.0f, 0.0f);
  m_buffer_write_to_perform.mesh[3] = Vertex(m_position, w, 0.0f, 1.0f, 0.0f);
  m_buffer_write_to_perform.mesh[4] =
      Vertex(m_position, 0.0f, 0.0f, 0.0f, 0.0f);
  m_buffer_write_to_perform.mesh[5] = Vertex(m_position, 0.0f, h, 0.0f, 1.0f);

  m_buffer_write_to_perform.image_indices.clear();
  for (uint32_t i = 0; i < m_buffers.size(); i++) {
    m_buffer_write_to_perform.image_indices.emplace(i);
  }
}
} // namespace text
} // namespace core
