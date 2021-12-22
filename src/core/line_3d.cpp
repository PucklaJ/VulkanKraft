#include "line_3d.hpp"
#include "exception.hpp"
#include <glm/gtx/transform.hpp>

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
  m_shader->set_push_constant(render_call, m_model_matrix);
  render_call.render_vertices(m_num_vertices);
}

void Line3D::set_model_matrix(const glm::vec3 &position) {
  m_model_matrix = glm::translate(position);
}

Shader *Line3D::m_shader = nullptr;

} // namespace core