#pragma once
#include "context.hpp"
#include "render_call.hpp"
#include <vector>

namespace core {
namespace vulkan {

// Holds a SPV shader module
struct SPVData {
  // Pointer to the memory
  const uint8_t *data;
  // Size in bytes of the memory
  size_t size;
};

// This class represents a vulkan pipeline used for graphics rendering (used for
// shaders)
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

  // Bind the pipeline before executing graphics commands
  void bind(const RenderCall &render_call) const noexcept;
  inline const vk::PipelineLayout &get_layout() const { return m_layout; }

private:
  // Name of the main function in the glsl shader code
  static constexpr char _shader_function_name[] = "main";

  // Create a vulkan shader module from SPVData
  static vk::ShaderModule _create_shader_module(const vk::Device &device,
                                                const SPVData &shader_code);

  // Create the vulkan pipeline handle
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