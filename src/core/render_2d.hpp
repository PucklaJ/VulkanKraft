#pragma once
#include "shader.hpp"
#include "texture.hpp"
#include "vulkan/render_call.hpp"
#include <glm/glm.hpp>

namespace core {
class Render2D {
public:
  Render2D(Texture &texture, Shader &shader);

  void set_model_matrix(const glm::vec2 &position = glm::vec2(0.0f, 0.0f),
                        const glm::vec2 &scale = glm::vec2(1.0f, 1.0f));
  void update_projection_matrix(const uint32_t window_width,
                                const uint32_t window_height,
                                const vulkan::RenderCall &render_call) const;
  void render(const vulkan::RenderCall &render_call);

private:
  glm::mat4 m_model_matrix;

  Texture &m_texture;
  Shader &m_shader;
};
} // namespace core
