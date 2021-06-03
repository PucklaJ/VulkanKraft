#pragma once
#include "context.hpp"
#include "render_call.hpp"
#include <vector>

namespace core {
namespace vulkan {
class GraphicsPipeline {
public:
  GraphicsPipeline(
      const Context &context,
      std::vector<vk::DescriptorSetLayout> descriptor_set_layouts,
      const std::vector<uint8_t> &vertex_code,
      const std::vector<uint8_t> &fragment_code,
      vk::VertexInputBindingDescription vertex_binding,
      std::vector<vk::VertexInputAttributeDescription> vertex_attributes,
      const vk::SampleCountFlagBits msaa_samples);
  ~GraphicsPipeline();

  void bind(const RenderCall &render_call) const noexcept;
  inline const vk::PipelineLayout &get_layout() const { return m_layout; }

private:
  static constexpr char _shader_function_name[] = "main";

  static vk::ShaderModule
  _create_shader_module(const vk::Device &device,
                        const std::vector<uint8_t> &shader_code);

  void _create_handle(
      std::vector<vk::DescriptorSetLayout> descriptor_set_layout,
      const std::vector<uint8_t> &vertex_code,
      const std::vector<uint8_t> &fragment_code,
      vk::VertexInputBindingDescription vertex_binding,
      std::vector<vk::VertexInputAttributeDescription> vertex_attributes,
      const vk::SampleCountFlagBits msaa_samples);
  void _destroy();

  vk::Pipeline m_handle;
  vk::PipelineLayout m_layout;
  vk::ShaderModule m_vertex_module;
  vk::ShaderModule m_fragment_module;

  const Context &m_context;
};
} // namespace vulkan
} // namespace core