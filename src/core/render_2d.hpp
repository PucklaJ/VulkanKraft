#pragma once
#include "shader.hpp"
#include "texture.hpp"
#include "vulkan/render_call.hpp"
#include <glm/glm.hpp>

namespace core {
// An object that handles the rendering of a texture as a 2D square
class Render2D {
public:
  // Needs to be called before using any objects of this class
  // This needs to be the texture 2d shader
  static inline void set_shader(Shader &shader) { m_shader = &shader; }
  // Binds the texture 2d shader
  static inline void bind_shader(const vulkan::RenderCall &render_call) {
    m_shader->bind(render_call);
  }
  // Calculates the orthographic projection for 2d and uploads it to the GPU
  static glm::mat4
  update_projection_matrix(const uint32_t window_width,
                           const uint32_t window_height,
                           const vulkan::RenderCall &render_call);

  Render2D();
  Render2D(Texture *texture);

  inline void set_texture(Texture *texture) { m_texture = texture; }

  // Updates the model matrix
  void set_model_matrix(const glm::vec2 &position = glm::vec2(0.0f, 0.0f),
                        const glm::vec2 &scale = glm::vec2(1.0f, 1.0f));
  // Renders the texture to the screen
  void render(const vulkan::RenderCall &render_call);

private:
  static Shader *m_shader;

  glm::mat4 m_model_matrix;

  Texture *m_texture;
};
} // namespace core
