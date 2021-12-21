#include "line_3d.hpp"
#include "exception.hpp"

namespace core {
void Line3D::begin() {
  m_current_vertices.clear();
  m_num_vertices = 0;
}

void Line3D::end(const vulkan::Context &context) {
  if (m_current_vertices.empty()) {
    throw VulkanKraftException("no vertices have been added to the Line3D");
  }

  if (!m_vertex_buffer) {
    m_vertex_buffer = std::make_unique<vulkan::Buffer>(
        context, vk::BufferUsageFlagBits::eVertexBuffer,
        m_current_vertices.size() * sizeof(Vertex), m_current_vertices.data());
  } else {
    m_vertex_buffer->set_data(m_current_vertices.data(),
                              m_current_vertices.size() * sizeof(Vertex));
  }

  m_num_vertices = static_cast<uint32_t>(m_current_vertices.size());
  m_current_vertices.clear();
}

void Line3D::render(const vulkan::RenderCall &render_call) {
  m_vertex_buffer->bind(render_call);
  render_call.render_vertices(m_num_vertices);
}

} // namespace core