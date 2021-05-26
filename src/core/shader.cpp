#include "shader.hpp"
#include "exception.hpp"
#include <fstream>
#include <glm/gtx/transform.hpp>

namespace core {
Shader::Builder::Builder() {}

Shader Shader::Builder::build(const vulkan::Context &context) {
  if (!m_vertex_path) {
    throw VulkanKraftException(
        "no vertex shader has been provided for core::Shader");
  }

  if (!m_fragment_path) {
    throw VulkanKraftException(
        "no fragment shader has been provided for core::Shader");
  }

  return Shader(context, m_vertex_path.value(), m_fragment_path.value(),
                std::move(m_uniform_buffers));
}

Shader::Builder &Shader::Builder::vertex(std::filesystem::path vertex_path) {
  m_vertex_path = std::move(vertex_path);
  return *this;
}

Shader::Builder &
Shader::Builder::fragment(std::filesystem::path fragment_path) {
  m_fragment_path = std::move(fragment_path);
  return *this;
}

Shader::Builder &
Shader::Builder::add_uniform_buffer(vk::ShaderStageFlags shader_stage) {
  m_uniform_buffers.push_back(shader_stage);
  return *this;
}

Shader::~Shader() {
  m_pipeline.reset();
  m_context.destroy_descriptors(std::move(m_descriptor_pool),
                                std::move(m_descriptor_layout));
}

void Shader::update_uniform_buffer(const MatrixData &data, const size_t index) {
  if (!m_context.get_current_swap_chain_image()) {
    return;
  }

  m_uniform_buffers[m_context.get_current_swap_chain_image().value()][index]
      .set_data(&data, sizeof(MatrixData));
}

void Shader::bind() {
  m_pipeline->bind();
  m_context.bind_descriptor_set(m_descriptor_sets, m_pipeline->get_layout());
}

std::vector<char> Shader::_read_spv_file(std::filesystem::path file_name) {
  std::ifstream file;
  file.open(file_name, std::ios_base::ate | std::ios_base::binary);
  if (file.fail()) {
    throw VulkanKraftException("failed to open shader file " +
                               file_name.string());
  }

  const auto file_size{file.tellg()};
  file.seekg(0, std::ios_base::seekdir::_S_beg);
  std::vector<char> data(static_cast<size_t>(file_size));
  file.read(data.data(), file_size);

  return data;
}

Shader::Shader(const vulkan::Context &context,
               std::filesystem::path vertex_path,
               std::filesystem::path fragment_path,
               std::vector<vk::ShaderStageFlags> uniform_buffers)
    : m_context(context) {

  _create_graphics_pipeline(context, std::move(vertex_path),
                            std::move(fragment_path), uniform_buffers);
  _create_descriptor_pool(context, uniform_buffers);
  _create_uniform_buffers(context, std::move(uniform_buffers));
  _create_descriptor_sets(context);
}

void Shader::_create_graphics_pipeline(
    const vulkan::Context &context, std::filesystem::path vertex_path,
    std::filesystem::path fragment_path,
    const std::vector<vk::ShaderStageFlags> &uniform_buffers) {
  size_t binding_point{0};

  std::vector<vk::DescriptorSetLayoutBinding> bindings(uniform_buffers.size());
  for (size_t i = 0; i < bindings.size(); i++) {
    bindings[i].binding = binding_point++;
    bindings[i].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = uniform_buffers[i];
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

  auto vertex_code = _read_spv_file(std::move(vertex_path));
  auto fragment_code = _read_spv_file(std::move(fragment_path));

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
    const std::vector<vk::ShaderStageFlags> &uniform_buffers) {
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
    std::vector<vk::ShaderStageFlags> uniform_buffers) {
  MatrixData data;
  data.model = glm::identity<glm::mat4>();
  data.view = glm::identity<glm::mat4>();
  data.proj = glm::identity<glm::mat4>();
  for (size_t i = 0; i < context.get_swap_chain_image_count(); i++) {
    auto &current_buffers = m_uniform_buffers.emplace_back();
    for (size_t j = 0; j < uniform_buffers.size(); j++) {
      current_buffers.emplace_back(context,
                                   vk::BufferUsageFlagBits::eUniformBuffer,
                                   sizeof(MatrixData), &data);
    }
  }
}

void Shader::_create_descriptor_sets(const vulkan::Context &context) {
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
      bi.range = sizeof(MatrixData);

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
} // namespace core