#include "graphics_pipeline.hpp"
#include "../exception.hpp"
#include "vertex.hpp"
#include <array>

namespace core {
namespace vulkan {
GraphicsPipeline::GraphicsPipeline(
    const Context &context,
    std::vector<vk::DescriptorSetLayout> descriptor_set_layouts,
    const SPVData &vertex_code, const SPVData &fragment_code,
    vk::VertexInputBindingDescription vertex_binding,
    std::vector<vk::VertexInputAttributeDescription> vertex_attributes,
    const vk::SampleCountFlagBits msaa_samples, const bool alpha_blending,
    const std::vector<vk::PushConstantRange> &push_constant_ranges,
    const vk::PrimitiveTopology primitive_topology)
    : m_context(context) {
  _create_handle(std::move(descriptor_set_layouts), vertex_code, fragment_code,
                 std::move(vertex_binding), std::move(vertex_attributes),
                 msaa_samples, alpha_blending, push_constant_ranges,
                 primitive_topology);
}

GraphicsPipeline::~GraphicsPipeline() { _destroy(); }

void GraphicsPipeline::bind(const RenderCall &render_call) const noexcept {
  render_call.bind_graphics_pipeline(m_handle);
}

vk::ShaderModule
GraphicsPipeline::_create_shader_module(const vk::Device &device,
                                        const SPVData &shader_code) {
  vk::ShaderModuleCreateInfo si;
  si.codeSize = shader_code.size;
  si.pCode = reinterpret_cast<const uint32_t *>(shader_code.data);

  try {
    return device.createShaderModule(si);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create shader module: ") +
                               e.what());
  }
}

void GraphicsPipeline::_create_handle(
    std::vector<vk::DescriptorSetLayout> descriptor_set_layouts,
    const SPVData &vertex_code, const SPVData &fragment_code,
    vk::VertexInputBindingDescription vertex_binding,
    std::vector<vk::VertexInputAttributeDescription> vertex_attributes,
    const vk::SampleCountFlagBits msaa_samples, const bool alpha_blending,
    const std::vector<vk::PushConstantRange> &push_constant_ranges,
    const vk::PrimitiveTopology primitive_topology) {
  try {
    m_vertex_module =
        _create_shader_module(m_context.get_device(), vertex_code);
  } catch (const VulkanKraftException &e) {
    throw VulkanKraftException(
        std::string("failed to create vertex shader module: ") + e.what());
  }

  try {
    m_fragment_module =
        _create_shader_module(m_context.get_device(), fragment_code);
  } catch (const VulkanKraftException &e) {
    throw VulkanKraftException(
        std::string("failed to create fragment shader module: ") + e.what());
  }

  vk::PipelineShaderStageCreateInfo vert_i;
  vert_i.stage = vk::ShaderStageFlagBits::eVertex;
  vert_i.module = m_vertex_module;
  vert_i.pName = _shader_function_name;

  vk::PipelineShaderStageCreateInfo frag_i;
  frag_i.stage = vk::ShaderStageFlagBits::eFragment;
  frag_i.module = m_fragment_module;
  frag_i.pName = _shader_function_name;

  const auto shader_stages = std::array{vert_i, frag_i};

  vk::PipelineVertexInputStateCreateInfo vi_i;
  vi_i.vertexBindingDescriptionCount = !vertex_attributes.empty();
  vi_i.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertex_attributes.size());
  vi_i.pVertexBindingDescriptions = &vertex_binding;
  vi_i.pVertexAttributeDescriptions = vertex_attributes.data();

  vk::PipelineInputAssemblyStateCreateInfo ia_i;
  ia_i.topology = primitive_topology;
  ia_i.primitiveRestartEnable = VK_FALSE;

  vk::PipelineViewportStateCreateInfo vs_i;
  vs_i.viewportCount = 1;
  vs_i.scissorCount = 1;

  vk::PipelineRasterizationStateCreateInfo rast_i;
  rast_i.depthClampEnable = VK_FALSE;
  rast_i.rasterizerDiscardEnable = VK_FALSE;
  rast_i.polygonMode = vk::PolygonMode::eFill;
  rast_i.lineWidth = 1.0f;
  rast_i.cullMode = vk::CullModeFlagBits::eBack;
  rast_i.frontFace = vk::FrontFace::eCounterClockwise;
  rast_i.depthBiasEnable = VK_FALSE;

  vk::PipelineMultisampleStateCreateInfo multi_i;
  multi_i.sampleShadingEnable =
      static_cast<vk::Bool32>(msaa_samples != vk::SampleCountFlagBits::e1);
  multi_i.rasterizationSamples = msaa_samples;
  multi_i.sampleShadingEnable = VK_FALSE;

  vk::PipelineColorBlendAttachmentState col_blend_at;
  col_blend_at.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  if (alpha_blending) {
    col_blend_at.blendEnable = VK_TRUE;
    col_blend_at.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    col_blend_at.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    col_blend_at.colorBlendOp = vk::BlendOp::eAdd;
    col_blend_at.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    col_blend_at.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    col_blend_at.alphaBlendOp = vk::BlendOp::eAdd;
  } else {
    col_blend_at.blendEnable = VK_FALSE;
  }

  vk::PipelineColorBlendStateCreateInfo cb_i;
  cb_i.logicOpEnable = VK_FALSE;
  cb_i.logicOp = vk::LogicOp::eCopy;
  cb_i.attachmentCount = 1;
  cb_i.pAttachments = &col_blend_at;
  cb_i.blendConstants.fill(0.0f);

  vk::PipelineLayoutCreateInfo pl_i;
  pl_i.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  pl_i.pSetLayouts = descriptor_set_layouts.data();
  pl_i.pPushConstantRanges = push_constant_ranges.data();
  pl_i.pushConstantRangeCount =
      static_cast<uint32_t>(push_constant_ranges.size());

  try {
    m_layout = m_context.get_device().createPipelineLayout(pl_i);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create pipeline layout: ") + e.what());
  }

  vk::PipelineDepthStencilStateCreateInfo ds_i;
  ds_i.depthTestEnable = VK_TRUE;
  ds_i.depthWriteEnable = VK_TRUE;
  ds_i.depthCompareOp = vk::CompareOp::eLess;
  ds_i.depthBoundsTestEnable = VK_FALSE;
  ds_i.minDepthBounds = 0.0f;
  ds_i.maxDepthBounds = 1.0f;
  ds_i.stencilTestEnable = VK_FALSE;

  const auto dyn_states =
      std::array{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dyn_i;
  dyn_i.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
  dyn_i.pDynamicStates = dyn_states.data();

  vk::GraphicsPipelineCreateInfo p_i;
  p_i.stageCount = static_cast<uint32_t>(shader_stages.size());
  p_i.pStages = shader_stages.data();
  p_i.pVertexInputState = &vi_i;
  p_i.pInputAssemblyState = &ia_i;
  p_i.pViewportState = &vs_i;
  p_i.pRasterizationState = &rast_i;
  p_i.pMultisampleState = &multi_i;
  p_i.pColorBlendState = &cb_i;
  p_i.pDepthStencilState = &ds_i;
  p_i.pDynamicState = &dyn_i;
  p_i.layout = m_layout;
  p_i.renderPass = m_context.get_swap_chain_render_pass();
  p_i.subpass = 0;
  p_i.basePipelineHandle = VK_NULL_HANDLE;

  try {
    const auto ps =
        m_context.get_device().createGraphicsPipelines(VK_NULL_HANDLE, p_i);
    if (ps.result != vk::Result::eSuccess) {
      throw std::runtime_error(std::to_string(static_cast<int>(ps.result)));
    }
    m_handle = ps.value[0];
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create graphics pipeline: ") + e.what());
  }
}

void GraphicsPipeline::_destroy() {
  m_context.get_device().waitIdle();

  m_context.get_device().destroyPipeline(m_handle);
  m_context.get_device().destroyPipelineLayout(m_layout);
  m_context.get_device().destroyShaderModule(m_vertex_module);
  m_context.get_device().destroyShaderModule(m_fragment_module);
}

} // namespace vulkan
} // namespace core