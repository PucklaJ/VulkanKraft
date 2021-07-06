#pragma once
#include "context.hpp"
#include "render_call.hpp"
#include <vector>

namespace core {
namespace vulkan {

struct SPVData {
  const uint8_t *data;
  size_t size;
};

class GraphicsPipeline {
public:
  GraphicsPipeline(
      const Context &context,
      std::vector<vk::DescriptorSetLayout> descriptor_set_layouts,
      const SPVData &vertex_code, const SPVData &fragment_code,
      vk::VertexInputBindingDescription vertex_binding,
      std::vector<vk::VertexInputAttributeDescription> vertex_attributes,
      const vk::SampleCountFlagBits msaa_samples, const bool alpha_blending);
  ~GraphicsPipeline();

  void bind(const RenderCall &render_call) const noexcept;
  inline const vk::PipelineLayout &get_layout() const { return m_layout; }

private:
  static constexpr char _shader_function_name[] = "main";

  static vk::ShaderModule _create_shader_module(const vk::Device &device,
                                                const SPVData &shader_code);

  void _create_handle(
      std::vector<vk::DescriptorSetLayout> descriptor_set_layout,
      const SPVData &vertex_code, const SPVData &fragment_code,
      vk::VertexInputBindingDescription vertex_binding,
      std::vector<vk::VertexInputAttributeDescription> vertex_attributes,
      const vk::SampleCountFlagBits msaa_samples, const bool alpha_blending);
  void _destroy();

  vk::Pipeline m_handle;
  vk::PipelineLayout m_layout;
  vk::ShaderModule m_vertex_module;
  vk::ShaderModule m_fragment_module;

  const Context &m_context;
};
} // namespace vulkan
} // namespace core