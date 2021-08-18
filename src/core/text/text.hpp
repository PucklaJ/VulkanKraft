#pragma once
#include "../shader.hpp"
#include "../texture.hpp"
#include "../vulkan/buffer.hpp"
#include "../vulkan/render_call.hpp"
#include "font.hpp"
#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <set>

namespace core {
namespace text {
// This represents a 2D text rendered to the screen
class Text {
public:
  // A value used by shader which unfortunately limits how much text objects can
  // be used at the same time
  static constexpr uint32_t max_texts = 20;

  // Holds all global uniforms used in the vertex shader
  struct GlobalUniform {
    glm::mat4 proj;
  };

  // The Vertex used by the text shader
  class Vertex {
  public:
    Vertex() = default;
    // lt ..... left top position of the rectangle
    Vertex(const glm::vec2 &lt, glm::f32 x, glm::f32 y, glm::f32 u, glm::f32 v)
        : position(lt.x + x, lt.y + y), uv(u, v) {}

    glm::vec2 position;
    glm::vec2 uv;
  };

  // The mesh used for the text objects will always hold six vertices
  typedef std::array<Vertex, 6> Mesh;

  // font ..... The font which should be used by this text object
  // string ... The initial text which should be displayed
  // position . The initial position
  Text(const vulkan::Context &context, Shader &shader, Font &font,
       const std::wstring &string, const glm::vec2 &position = glm::vec2(),
       const float font_size = 50.0f);

  void set_string(const std::wstring &string);
  void set_font_size(const float font_size);
  void set_position(const glm::vec2 &position);

  inline const uint32_t &get_width() const { return m_texture_width; }
  inline const uint32_t &get_height() const { return m_texture_height; }

  // Renders the text object to the screen
  void render(const vulkan::RenderCall &render_call);

private:
  // Holds the new mesh and which swap image indices still need the update
  struct BufferWrite {
    Mesh mesh;
    std::set<uint32_t> image_indices;
  };

  // Creates a new texture from the given string and font_size
  Texture _build_texture();
  // Creates a new mesh from the given position and texture size
  void _build_buffers();

  const vulkan::Context &m_context;
  Shader &m_shader;

  Font &m_font;
  std::wstring m_string;
  float m_font_size;
  glm::vec2 m_position;

  Texture m_text_texture;
  // There is a buffer for every swap chain image
  std::vector<vulkan::Buffer> m_buffers;
  uint32_t m_texture_width;
  uint32_t m_texture_height;
  BufferWrite m_buffer_write_to_perform;
};
} // namespace text
} // namespace core
