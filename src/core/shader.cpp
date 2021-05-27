#include "shader.hpp"
#include "exception.hpp"
#include <fstream>
#include <glm/gtx/transform.hpp>

namespace core {
Shader::Builder::Builder() {}

Shader Shader::Builder::build(const vulkan::Context &context) {
  if (m_vertex_code.empty()) {
    throw VulkanKraftException(
        "no vertex shader has been provided for core::Shader");
  }

  if (m_fragment_code.empty()) {
    throw VulkanKraftException(
        "no fragment shader has been provided for core::Shader");
  }

  return Shader(context, std::move(m_vertex_code), std::move(m_fragment_code),
                std::move(m_uniform_buffers));
}

std::vector<uint8_t>
Shader::Builder::_read_spv_file(std::filesystem::path file_name) {
  std::ifstream file;
  file.open(file_name, std::ios_base::ate | std::ios_base::binary);
  if (file.fail()) {
    throw VulkanKraftException("failed to open shader file " +
                               file_name.string());
  }

  const auto file_size{file.tellg()};
  file.seekg(0);
  std::vector<uint8_t> data(static_cast<size_t>(file_size));
  file.read(reinterpret_cast<char *>(data.data()), file_size);

  return data;
}

Shader::~Shader() {
  m_pipeline.reset();
  m_context.destroy_descriptors(std::move(m_descriptor_pool),
                                std::move(m_descriptor_layout));
}

void Shader::bind(const vulkan::RenderCall &render_call) {
  m_pipeline->bind(render_call);
  render_call.bind_descriptor_set(
      m_descriptor_sets[render_call.get_swap_chain_image_index()],
      m_pipeline->get_layout());
}

Shader::Shader(const vulkan::Context &context, std::vector<uint8_t> vertex_code,
               std::vector<uint8_t> fragment_code,
               std::vector<Builder::UniformBufferInfo> uniform_buffers)
    : m_context(context) {

  _create_graphics_pipeline(context, std::move(vertex_code),
                            std::move(fragment_code), uniform_buffers);
  _create_descriptor_pool(context, uniform_buffers);
  _create_uniform_buffers(context, uniform_buffers);
  _create_descriptor_sets(context, std::move(uniform_buffers));
}

void Shader::_create_graphics_pipeline(
    const vulkan::Context &context, std::vector<uint8_t> vertex_code,
    std::vector<uint8_t> fragment_code,
    const std::vector<Builder::UniformBufferInfo> &uniform_buffers) {
  size_t binding_point{0};

  std::vector<vk::DescriptorSetLayoutBinding> bindings(uniform_buffers.size());
  for (size_t i = 0; i < bindings.size(); i++) {
    bindings[i].binding = binding_point++;
    bindings[i].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = uniform_buffers[i].shader_stage;
    bindings[i].pImmutableSamplers = nullptr;
  }

  try {
    m_descriptor_layout =
        context.create_descriptor_set_layout(std::move(bindings));
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to create descriptor set layout for core::Shader: ") +
        e.what());
  }

  try {
    m_pipeline = std::make_unique<vulkan::GraphicsPipeline>(
        context, m_descriptor_layout, std::move(vertex_code),
        std::move(fragment_code));
  } catch (const VulkanKraftException &e) {
    throw VulkanKraftException(
        std::string("failed to create graphics pipeline of core::Shader: ") +
        e.what());
  }
}

void Shader::_create_descriptor_pool(
    const vulkan::Context &context,
    const std::vector<Builder::UniformBufferInfo> &uniform_buffers) {
  std::vector<vk::DescriptorPoolSize> pool_sizes(uniform_buffers.size());
  for (auto &p : pool_sizes) {
    p.type = vk::DescriptorType::eUniformBuffer;
    p.descriptorCount =
        static_cast<uint32_t>(context.get_swap_chain_image_count());
  }

  try {
    m_descriptor_pool = context.create_descriptor_pool(
        std::move(pool_sizes), context.get_swap_chain_image_count());
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create descriptor pool of core::Shader: ") +
        e.what());
  }
}

void Shader::_create_uniform_buffers(
    const vulkan::Context &context,
    const std::vector<Builder::UniformBufferInfo> &uniform_buffers) {
  for (size_t i = 0; i < context.get_swap_chain_image_count(); i++) {
    auto &current_buffers = m_uniform_buffers.emplace_back();
    for (size_t j = 0; j < uniform_buffers.size(); j++) {
      current_buffers.emplace_back(context,
                                   vk::BufferUsageFlagBits::eUniformBuffer,
                                   uniform_buffers[j].initial_state.size(),
                                   uniform_buffers[j].initial_state.data());
    }
  }
}

void Shader::_create_descriptor_sets(
    const vulkan::Context &context,
    std::vector<Builder::UniformBufferInfo> uniform_buffers) {
  try {
    m_descriptor_sets =
        context.create_descriptor_sets(m_descriptor_pool, m_descriptor_layout);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to allocate descriptor sets for core::Shader: ") +
        e.what());
  }

  for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
    std::vector<vk::WriteDescriptorSet> writes;
    size_t current_binding{0};
    for (size_t j = 0; j < m_uniform_buffers[i].size();
         j++, current_binding++) {
      vk::DescriptorBufferInfo bi;
      bi.buffer = m_uniform_buffers[i][j].get_handle();
      bi.offset = 0;
      bi.range = uniform_buffers[j].initial_state.size();

      vk::WriteDescriptorSet w;
      w.dstSet = m_descriptor_sets[i];
      w.dstBinding = current_binding;
      w.dstArrayElement = 0;
      w.descriptorType = vk::DescriptorType::eUniformBuffer;
      w.descriptorCount = 1;
      w.pBufferInfo = &bi;

      writes.emplace_back(std::move(w));
    }
    context.write_descriptor_sets(std::move(writes));
  }
}

void Shader::_update_uniform_buffer(const vulkan::RenderCall &render_call,
                                    const void *data, const size_t data_size,
                                    const size_t index) {
  m_uniform_buffers[render_call.get_swap_chain_image_index()][index].set_data(
      data, data_size);
}
} // namespace core