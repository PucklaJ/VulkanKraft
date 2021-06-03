#pragma once
#include "../shader.hpp"
#include "../texture.hpp"
#include "../vulkan/buffer.hpp"
#include "../vulkan/render_call.hpp"
#include "font.hpp"
#include <array>
#include <memory>
#include <optional>
#include <set>

namespace core {
namespace text {
class Text {
public:
  struct GlobalUniform {
    glm::mat4 proj;
  };

  class Vertex {
  public:
    Vertex() = default;
    Vertex(glm::f32 x, glm::f32 y, glm::f32 u, glm::f32 v)
        : position(x, y), uv(u, v) {}

    glm::vec2 position;
    glm::vec2 uv;
  };
  typedef std::array<Vertex, 6> Mesh;

  static Shader build_shader(const vulkan::Context &context,
                             const Settings &settings);

  Text(const vulkan::Context &context, Shader &shader, Font &font,
       const std::wstring &string, const float font_size = 50.0f);

  void set_string(const std::wstring &string);
  void set_font_size(const float font_size);
  void render(const vulkan::RenderCall &render_call);

private:
  struct BufferWrite {
    Mesh mesh;
    std::set<uint32_t> image_indices;
  };

  Texture _build_texture();
  void _build_buffers();

  const vulkan::Context &m_context;
  Shader &m_shader;

  Font &m_font;
  std::wstring m_string;
  float m_font_size;

  Texture m_text_texture;
  std::vector<vulkan::Buffer> m_buffers;
  uint32_t m_texture_width;
  uint32_t m_texture_height;
  BufferWrite m_buffer_write_to_perform;
};
} // namespace text
} // namespace core
