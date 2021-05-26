#pragma once
#include "context.hpp"
#include <vector>

namespace core {
namespace vulkan {
class GraphicsPipeline {
public:
  GraphicsPipeline(const Context &context,
                   vk::DescriptorSetLayout descriptor_set_layout,
                   std::vector<char> vertex_code,
                   std::vector<char> fragment_code);
  ~GraphicsPipeline();

  void bind();
  inline const vk::PipelineLayout &get_layout() const { return m_layout; }

private:
  static constexpr char _shader_function_name[] = "main";

  static vk::ShaderModule _create_shader_module(const vk::Device &device,
                                                std::vector<char> shader_code);

  void _create_handle(vk::DescriptorSetLayout descriptor_set_layout,
                      std::vector<char> vertex_code,
                      std::vector<char> fragment_code);
  void _destroy();

  vk::Pipeline m_handle;
  vk::PipelineLayout m_layout;
  vk::ShaderModule m_vertex_module;
  vk::ShaderModule m_fragment_module;

  const Context &m_context;
};
} // namespace vulkan
} // namespace core