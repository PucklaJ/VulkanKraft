#pragma once
#include "shader.hpp"
#include "texture.hpp"
#include "vulkan/render_call.hpp"
#include <glm/glm.hpp>

namespace core {
class Render2D {
public:
  static inline void set_shader(Shader &shader) { m_shader = &shader; }
  static inline void bind_shader(const vulkan::RenderCall &render_call) {
    m_shader->bind(render_call);
  }
  static glm::mat4
  update_projection_matrix(const uint32_t window_width,
                           const uint32_t window_height,
                           const vulkan::RenderCall &render_call);

  Render2D(Texture &texture);

  void set_model_matrix(const glm::vec2 &position = glm::vec2(0.0f, 0.0f),
                        const glm::vec2 &scale = glm::vec2(1.0f, 1.0f));
  void render(const vulkan::RenderCall &render_call);

private:
  static Shader *m_shader;

  glm::mat4 m_model_matrix;

  Texture &m_texture;
};
} // namespace core
