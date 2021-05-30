#include "shader.hpp"
#include "exception.hpp"
#include <fstream>

namespace core {
Shader::Builder::Builder() {}

Shader Shader::Builder::build(const vulkan::Context &context,
                              const Settings &settings) {
  if (m_vertex_code.empty()) {
    throw VulkanKraftException(
        "no vertex shader has been provided for core::Shader");
  }

  if (m_fragment_code.empty()) {
    throw VulkanKraftException(
        "no fragment shader has been provided for core::Shader");
  }

  if (m_vertex_attributes.empty()) {
    throw VulkanKraftException(
        "no vertex atrributes have been specified for core::Shader");
  }

  return Shader(context, settings, std::move(m_vertex_code),
                std::move(m_fragment_code), std::move(m_uniform_buffers),
                std::move(m_vertex_attributes), std::move(m_textures));
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

Shader::Shader(Shader &&rhs)
    : m_pipeline(std::move(rhs.m_pipeline)),
      m_uniform_buffers(std::move(rhs.m_uniform_buffers)),
      m_descriptor_layout(std::move(rhs.m_descriptor_layout)),
      m_descriptor_pool(std::move(rhs.m_descriptor_pool)),
      m_descriptor_sets(std::move(rhs.m_descriptor_sets)),
      m_context(rhs.m_context) {
  rhs.m_uniform_buffers.clear();
  rhs.m_descriptor_layout = VK_NULL_HANDLE;
  rhs.m_descriptor_pool = VK_NULL_HANDLE;
  rhs.m_descriptor_sets.clear();
}

Shader::~Shader() {
  m_pipeline.reset();
  if (m_descriptor_pool)
    m_context.get_device().destroyDescriptorPool(m_descriptor_pool);
  if (m_descriptor_layout)
    m_context.get_device().destroyDescriptorSetLayout(m_descriptor_layout);
}

void Shader::bind(const vulkan::RenderCall &render_call) {
  m_pipeline->bind(render_call);
  if (!m_descriptor_sets.empty())
    render_call.bind_descriptor_set(
        m_descriptor_sets[render_call.get_swap_chain_image_index()],
        m_pipeline->get_layout());
}

Shader::Shader(const vulkan::Context &context, const Settings &settings,
               std::vector<uint8_t> vertex_code,
               std::vector<uint8_t> fragment_code,
               std::vector<Builder::UniformBufferInfo> uniform_buffers,
               std::vector<Builder::VertexAttributeInfo> vertex_attributes,
               std::vector<const Texture *> textures)
    : m_context(context) {
  _create_graphics_pipeline(context, settings, std::move(vertex_code),
                            std::move(fragment_code), uniform_buffers,
                            std::move(vertex_attributes), textures);
  _create_descriptor_pool(context, uniform_buffers, textures);
  _create_uniform_buffers(context, uniform_buffers);
  _create_descriptor_sets(context, std::move(uniform_buffers),
                          std::move(textures));
}

void Shader::_create_graphics_pipeline(
    const vulkan::Context &context, const Settings &settings,
    std::vector<uint8_t> vertex_code, std::vector<uint8_t> fragment_code,
    const std::vector<Builder::UniformBufferInfo> &uniform_buffers,
    std::vector<Builder::VertexAttributeInfo> vertex_attributes,
    const std::vector<const Texture *> &textures) {
  size_t binding_point{0};

  std::vector<vk::DescriptorSetLayoutBinding> bindings(uniform_buffers.size() +
                                                       textures.size());
  for (size_t i = 0; i < uniform_buffers.size(); i++) {
    bindings[i].binding = binding_point++;
    bindings[i].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = uniform_buffers[i].shader_stage;
    bindings[i].pImmutableSamplers = nullptr;
  }
  for (size_t i = 0; i < textures.size(); i++, binding_point++) {
    bindings[binding_point].binding = binding_point;
    bindings[binding_point].descriptorType =
        vk::DescriptorType::eCombinedImageSampler;
    bindings[binding_point].descriptorCount = 1;
    bindings[binding_point].stageFlags = vk::ShaderStageFlagBits::eFragment;
    bindings[binding_point].pImmutableSamplers = nullptr;
  }

  vk::DescriptorSetLayoutCreateInfo li;
  li.bindingCount = static_cast<uint32_t>(bindings.size());
  li.pBindings = bindings.data();

  try {
    m_descriptor_layout = m_context.get_device().createDescriptorSetLayout(li);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to create descriptor set layout for core::Shader: ") +
        e.what());
  }

  vk::VertexInputBindingDescription bind;
  bind.binding = 0;
  for (const auto &vi : vertex_attributes) {
    bind.stride += static_cast<uint32_t>(vi.size);
  }
  bind.inputRate = vk::VertexInputRate::eVertex;

  std::vector<vk::VertexInputAttributeDescription> atts(
      vertex_attributes.size());
  for (size_t i = 0; i < atts.size(); i++) {
    atts[i].binding = 0;
    atts[i].location = static_cast<uint32_t>(i);
    atts[i].format = vertex_attributes[i].format;
    atts[i].offset =
        i == 0 ? 0
               : (atts[i - 1].offset +
                  static_cast<uint32_t>(vertex_attributes[i - 1].size));
  }

  try {
    m_pipeline = std::make_unique<vulkan::GraphicsPipeline>(
        context, m_descriptor_layout, std::move(vertex_code),
        std::move(fragment_code), std::move(bind), std::move(atts),
        settings.msaa_samples);
  } catch (const VulkanKraftException &e) {
    throw VulkanKraftException(
        std::string("failed to create graphics pipeline of core::Shader: ") +
        e.what());
  }
}

void Shader::_create_descriptor_pool(
    const vulkan::Context &context,
    const std::vector<Builder::UniformBufferInfo> &uniform_buffers,
    const std::vector<const Texture *> &textures) {
  if (uniform_buffers.empty() && textures.empty()) {
    return;
  }
  std::vector<vk::DescriptorPoolSize> pool_sizes(uniform_buffers.size() +
                                                 textures.size());
  for (size_t i = 0; i < uniform_buffers.size(); i++) {
    pool_sizes[i].type = vk::DescriptorType::eUniformBuffer;
    pool_sizes[i].descriptorCount =
        static_cast<uint32_t>(context.get_swap_chain_image_count());
  }
  for (size_t i = 0; i < textures.size(); i++) {
    pool_sizes[i + uniform_buffers.size()].type =
        vk::DescriptorType::eCombinedImageSampler;
    pool_sizes[i + uniform_buffers.size()].descriptorCount =
        static_cast<uint32_t>(context.get_swap_chain_image_count());
  }

  vk::DescriptorPoolCreateInfo pi;
  pi.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pi.pPoolSizes = pool_sizes.data();
  pi.maxSets = static_cast<uint32_t>(m_context.get_swap_chain_image_count());

  try {
    m_descriptor_pool = m_context.get_device().createDescriptorPool(pi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create descriptor pool of core::Shader: ") +
        e.what());
  }
}

void Shader::_create_uniform_buffers(
    const vulkan::Context &context,
    const std::vector<Builder::UniformBufferInfo> &uniform_buffers) {
  if (uniform_buffers.empty()) {
    return;
  }
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
    std::vector<Builder::UniformBufferInfo> uniform_buffers,
    std::vector<const Texture *> textures) {
  if (uniform_buffers.empty() && textures.empty()) {
    return;
  }

  const std::vector<vk::DescriptorSetLayout> layouts(
      m_context.get_swap_chain_image_count(), m_descriptor_layout);
  vk::DescriptorSetAllocateInfo ai;
  ai.descriptorPool = m_descriptor_pool;
  ai.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  ai.pSetLayouts = layouts.data();
  try {
    m_descriptor_sets = m_context.get_device().allocateDescriptorSets(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to allocate descriptor sets for core::Shader: ") +
        e.what());
  }

  for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
    std::vector<vk::WriteDescriptorSet> writes;
    std::vector<vk::DescriptorBufferInfo> buffer_infos(uniform_buffers.size());
    std::vector<vk::DescriptorImageInfo> image_infos(textures.size());
    size_t current_binding{0};
    for (size_t j = 0; j < m_uniform_buffers[i].size();
         j++, current_binding++) {
      vk::DescriptorBufferInfo bi;
      bi.buffer = m_uniform_buffers[i][j].get_handle();
      bi.offset = 0;
      bi.range = uniform_buffers[j].initial_state.size();
      buffer_infos[j] = std::move(bi);

      vk::WriteDescriptorSet w;
      w.dstSet = m_descriptor_sets[i];
      w.dstBinding = current_binding;
      w.dstArrayElement = 0;
      w.descriptorType = vk::DescriptorType::eUniformBuffer;
      w.descriptorCount = 1;
      w.pBufferInfo = &buffer_infos[j];

      writes.emplace_back(std::move(w));
    }
    for (size_t j = 0; j < textures.size(); j++, current_binding++) {
      auto ii(textures[j]->create_descriptor_image_info());
      image_infos[j] = std::move(ii);

      vk::WriteDescriptorSet w;
      w.dstSet = m_descriptor_sets[i];
      w.dstBinding = current_binding;
      w.dstArrayElement = 0;
      w.descriptorType = vk::DescriptorType::eCombinedImageSampler;
      w.descriptorCount = 1;
      w.pImageInfo = &image_infos[j];

      writes.emplace_back(std::move(w));
    }
    m_context.get_device().updateDescriptorSets(writes, nullptr);
  }
}

void Shader::_update_uniform_buffer(const vulkan::RenderCall &render_call,
                                    const void *data, const size_t data_size,
                                    const size_t index) {
  m_uniform_buffers[render_call.get_swap_chain_image_index()][index].set_data(
      data, data_size);
}
} // namespace core