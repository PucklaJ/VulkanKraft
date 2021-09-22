#include "render_2d.hpp"
#include <glm/gtx/transform.hpp>

namespace core {

glm::mat4
Render2D::update_projection_matrix(const uint32_t window_width,
                                   const uint32_t window_height,
                                   const vulkan::RenderCall &render_call) {
  const auto proj(glm::ortho(0.0f, static_cast<float>(window_width), 0.0f,
                             static_cast<float>(window_height)));
  m_shader->update_uniform_buffer(render_call, proj);
  return proj;
}

Render2D::Render2D() : m_texture(nullptr) { m_model_matrix = glm::mat4(1.0f); }

Render2D::Render2D(Texture *texture) : m_texture(texture) {
  set_model_matrix();
}

void Render2D::set_model_matrix(const glm::vec2 &position,
                                const glm::vec2 &scale) {
  m_model_matrix =
      glm::translate(glm::vec3(position.x, position.y, 0.0f)) *
      glm::scale(glm::vec3(
          static_cast<float>(m_texture->get_width() / 2) * scale.x,
          static_cast<float>(m_texture->get_height() / 2) * scale.y, 1.0f));
}

void Render2D::render(const vulkan::RenderCall &render_call) {
  m_shader->bind_dynamic_texture(render_call, *m_texture);
  m_shader->set_push_constant(render_call, m_model_matrix);
  render_call.render_vertices(6);
}

Shader *Render2D::m_shader = nullptr;

} // namespace core
