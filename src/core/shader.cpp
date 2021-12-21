#include "shader.hpp"
#include "exception.hpp"
#include <fstream>

namespace core {
Shader::Builder::Builder()
    : m_texture_count(0),
      m_alpha_blending(false), m_vertex_code{0, 0}, m_fragment_code{0, 0},
      m_current_push_constant_offset(0),
      m_primitive_topology(vk::PrimitiveTopology::eTriangleList) {}

Shader Shader::Builder::build(const vulkan::Context &context,
                              const Settings &settings) {
  if (!m_vertex_code.data) {
    throw VulkanKraftException(
        "no vertex shader has been provided for core::Shader");
  }

  if (!m_fragment_code.data) {
    throw VulkanKraftException(
        "no fragment shader has been provided for core::Shader");
  }

  return Shader(context, settings, *this);
}

Shader::Shader(Shader &&rhs)
    : m_pipeline(std::move(rhs.m_pipeline)),
      m_uniform_buffers(std::move(rhs.m_uniform_buffers)),
      m_descriptor_layout(std::move(rhs.m_descriptor_layout)),
      m_dynamic_textures_layout(std::move(rhs.m_dynamic_textures_layout)),
      m_descriptor_pool(std::move(rhs.m_descriptor_pool)),
      m_descriptor_sets(std::move(rhs.m_descriptor_sets)),
      m_push_constants(std::move(rhs.m_push_constants)),
      m_texture_writes_to_perform(std::move(rhs.m_texture_writes_to_perform)),
      m_min_dynamic_texture_binding_point(
          rhs.m_min_dynamic_texture_binding_point),
      m_context(rhs.m_context) {
  rhs.m_uniform_buffers.clear();
  rhs.m_dynamic_textures_layout.clear();
  rhs.m_descriptor_layout = VK_NULL_HANDLE;
  rhs.m_descriptor_pool = VK_NULL_HANDLE;
  rhs.m_descriptor_sets.clear();
  rhs.m_texture_writes_to_perform.clear();
}

Shader::~Shader() {
  m_pipeline.reset();
  if (m_descriptor_pool)
    m_context.get_device().destroyDescriptorPool(m_descriptor_pool);
  if (m_descriptor_layout)
    m_context.get_device().destroyDescriptorSetLayout(m_descriptor_layout);
  for (auto &l : m_dynamic_textures_layout) {
    m_context.get_device().destroyDescriptorSetLayout(l);
  }
}

void Shader::set_texture(const Texture &texture, const size_t index) {
  auto &texture_write = m_texture_writes_to_perform[index];

  texture_write.image_info = texture.create_descriptor_image_info();

  texture_write.write.dstBinding =
      m_uniform_buffers.empty() ? index
                                : (m_uniform_buffers.front().size() + index);
  texture_write.write.descriptorCount = 1;
  texture_write.write.pImageInfo = &texture_write.image_info;
  texture_write.write.descriptorType =
      vk::DescriptorType::eCombinedImageSampler;
  texture_write.write.dstArrayElement = 0;

  texture_write.image_indices.clear();
  for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
    texture_write.image_indices.emplace(static_cast<uint32_t>(i));
  }
}

void Shader::bind_dynamic_texture(const vulkan::RenderCall &render_call,
                                  Texture &texture, const size_t index) const {
  if (m_min_dynamic_texture_binding_point == -1)
    throw VulkanKraftException(
        "tried to bind dynamic texture, but are none specified");

  texture._create_descriptor_sets(m_descriptor_pool,
                                  m_dynamic_textures_layout[index],
                                  index + m_min_dynamic_texture_binding_point);
  texture._write_dynamic_set(render_call.get_swap_chain_image_index());

  render_call.bind_descriptor_set(
      texture.m_dynamic_sets[render_call.get_swap_chain_image_index()],
      m_pipeline->get_layout(),
      index + std::min(m_min_dynamic_texture_binding_point,
                       static_cast<size_t>(1)));
}

void Shader::bind(const vulkan::RenderCall &render_call) {
  m_pipeline->bind(render_call);
  if (!m_descriptor_sets.empty()) {
    for (auto &[i, tw] : m_texture_writes_to_perform) {
      if (tw.image_indices.count(render_call.get_swap_chain_image_index()) !=
          0) {
        tw.write.dstSet =
            m_descriptor_sets[render_call.get_swap_chain_image_index()];

        m_context.get_device().updateDescriptorSets(tw.write, nullptr);

        tw.image_indices.erase(render_call.get_swap_chain_image_index());
      }
    }

    render_call.bind_descriptor_set(
        m_descriptor_sets[render_call.get_swap_chain_image_index()],
        m_pipeline->get_layout(), 0);
  }
}

Shader::Shader(const vulkan::Context &context, const Settings &settings,
               const Builder &builder)
    : m_min_dynamic_texture_binding_point(-1), m_context(context) {
  _create_graphics_pipeline(context, settings, builder);
  _create_descriptor_pool(context, builder);
  _create_uniform_buffers(context, builder);
  _create_descriptor_sets(context, builder);
}

void Shader::_create_graphics_pipeline(const vulkan::Context &context,
                                       const Settings &settings,
                                       const Builder &builder) {
  for (const auto &pc : builder.m_push_constants) {
    m_push_constants.emplace_back(PushConstantData{pc.stageFlags, pc.offset});
  }

  size_t binding_point{0};

  std::vector<std::vector<vk::DescriptorSetLayoutBinding>> all_bindings(
      std::min(static_cast<size_t>(1),
               builder.m_uniform_buffers.size() + builder.m_texture_count) +
      builder.m_dynamic_textures.size());
  size_t current_bindings = 0;
  if (builder.m_uniform_buffers.size() + builder.m_texture_count > 0) {
    auto &bindings = all_bindings[current_bindings];
    bindings.resize(builder.m_uniform_buffers.size() + builder.m_texture_count);
    for (size_t i = 0; i < builder.m_uniform_buffers.size(); i++) {
      bindings[i].binding = binding_point++;
      bindings[i].descriptorType = vk::DescriptorType::eUniformBuffer;
      bindings[i].descriptorCount = 1;
      bindings[i].stageFlags = builder.m_uniform_buffers[i].shader_stage;
      bindings[i].pImmutableSamplers = nullptr;
    }
    for (size_t i = 0; i < builder.m_texture_count; i++, binding_point++) {
      bindings[binding_point].binding = binding_point;
      bindings[binding_point].descriptorType =
          vk::DescriptorType::eCombinedImageSampler;
      bindings[binding_point].descriptorCount = 1;
      bindings[binding_point].stageFlags = vk::ShaderStageFlagBits::eFragment;
      bindings[binding_point].pImmutableSamplers = nullptr;
    }
    current_bindings++;
  }
  if (!builder.m_dynamic_textures.empty()) {
    m_min_dynamic_texture_binding_point = binding_point;
    for (size_t i = 0; i < builder.m_dynamic_textures.size();
         i++, binding_point++, current_bindings++) {
      auto &bindings = all_bindings[current_bindings];
      bindings.resize(1);

      bindings[0].binding = binding_point;
      bindings[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
      bindings[0].descriptorCount = 1;
      bindings[0].stageFlags = vk::ShaderStageFlagBits::eFragment;
      bindings[0].pImmutableSamplers = nullptr;
    }
  }

  current_bindings = 0;
  if (builder.m_uniform_buffers.size() + builder.m_texture_count > 0) {
    vk::DescriptorSetLayoutCreateInfo li;
    li.bindingCount = static_cast<uint32_t>(all_bindings.front().size());
    li.pBindings = all_bindings.front().data();

    try {
      m_descriptor_layout =
          m_context.get_device().createDescriptorSetLayout(li);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string(
              "failed to create descriptor set layout for core::Shader: ") +
          e.what());
    }
    current_bindings++;
  }

  m_dynamic_textures_layout.resize(builder.m_dynamic_textures.size());
  for (size_t i = 0; i < builder.m_dynamic_textures.size();
       i++, current_bindings++) {
    const auto &bindings = all_bindings[current_bindings];
    vk::DescriptorSetLayoutCreateInfo li;
    li.bindingCount = static_cast<uint32_t>(bindings.size());
    li.pBindings = bindings.data();

    try {
      auto l = m_context.get_device().createDescriptorSetLayout(li);
      m_dynamic_textures_layout[i] = l;
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string(
              "failed to create descriptor set layout for core::Shader: ") +
          e.what());
    }
  }

  vk::VertexInputBindingDescription bind;
  bind.binding = 0;
  for (const auto &vi : builder.m_vertex_attributes) {
    bind.stride += static_cast<uint32_t>(vi.size);
  }
  bind.inputRate = vk::VertexInputRate::eVertex;

  std::vector<vk::VertexInputAttributeDescription> atts(
      builder.m_vertex_attributes.size());
  for (size_t i = 0; i < atts.size(); i++) {
    atts[i].binding = 0;
    atts[i].location = static_cast<uint32_t>(i);
    atts[i].format = builder.m_vertex_attributes[i].format;
    atts[i].offset =
        i == 0
            ? 0
            : (atts[i - 1].offset +
               static_cast<uint32_t>(builder.m_vertex_attributes[i - 1].size));
  }

  std::vector<vk::DescriptorSetLayout> all_layouts;
  if (builder.m_uniform_buffers.size() + builder.m_texture_count > 0)
    all_layouts.push_back(m_descriptor_layout);
  all_layouts.reserve(all_layouts.size() + m_dynamic_textures_layout.size());
  for (const auto &l : m_dynamic_textures_layout) {
    all_layouts.push_back(l);
  }

  try {
    m_pipeline = std::make_unique<vulkan::GraphicsPipeline>(
        context, std::move(all_layouts), std::move(builder.m_vertex_code),
        std::move(builder.m_fragment_code), std::move(bind), std::move(atts),
        settings.msaa_samples, builder.m_alpha_blending,
        builder.m_push_constants, builder.m_primitive_topology);
  } catch (const VulkanKraftException &e) {
    throw VulkanKraftException(
        std::string("failed to create graphics pipeline of core::Shader: ") +
        e.what());
  }
}

void Shader::_create_descriptor_pool(const vulkan::Context &context,
                                     const Builder &builder) {
  if (builder.m_uniform_buffers.empty() && builder.m_texture_count == 0 &&
      builder.m_dynamic_textures.empty()) {
    return;
  }
  std::vector<vk::DescriptorPoolSize> pool_sizes;
  pool_sizes.reserve(2);
  if (!builder.m_uniform_buffers.empty()) {
    auto &ps = pool_sizes.emplace_back();
    ps.type = vk::DescriptorType::eUniformBuffer;
    ps.descriptorCount =
        static_cast<uint32_t>(context.get_swap_chain_image_count() *
                              builder.m_uniform_buffers.size());
  }
  if (builder.m_texture_count != 0 || !builder.m_dynamic_textures.empty()) {
    auto &ps = pool_sizes.emplace_back();
    ps.type = vk::DescriptorType::eCombinedImageSampler;
    ps.descriptorCount = static_cast<uint32_t>(
        context.get_swap_chain_image_count() * builder.m_texture_count);
    for (const auto &ms : builder.m_dynamic_textures) {
      ps.descriptorCount +=
          static_cast<uint32_t>(context.get_swap_chain_image_count() * ms);
    }
  }

  vk::DescriptorPoolCreateInfo pi;
  pi.flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  pi.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pi.pPoolSizes = pool_sizes.data();
  pi.maxSets = static_cast<uint32_t>(m_context.get_swap_chain_image_count());
  for (const auto &ms : builder.m_dynamic_textures) {
    pi.maxSets +=
        ms * static_cast<uint32_t>(m_context.get_swap_chain_image_count());
  }

  try {
    m_descriptor_pool = m_context.get_device().createDescriptorPool(pi);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create descriptor pool of core::Shader: ") +
        e.what());
  }
}

void Shader::_create_uniform_buffers(const vulkan::Context &context,
                                     const Builder &builder) {
  if (builder.m_uniform_buffers.empty()) {
    return;
  }
  for (size_t i = 0; i < context.get_swap_chain_image_count(); i++) {
    auto &current_buffers = m_uniform_buffers.emplace_back();
    for (size_t j = 0; j < builder.m_uniform_buffers.size(); j++) {
      current_buffers.emplace_back(
          context, vk::BufferUsageFlagBits::eUniformBuffer,
          builder.m_uniform_buffers[j].initial_state.size(),
          builder.m_uniform_buffers[j].initial_state.data());
    }
  }
}

void Shader::_create_descriptor_sets(const vulkan::Context &context,
                                     const Builder &builder) {
  if (builder.m_uniform_buffers.empty() && builder.m_texture_count == 0) {
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

  if (!m_uniform_buffers.empty()) {
    for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
      std::vector<vk::WriteDescriptorSet> writes;
      std::vector<vk::DescriptorBufferInfo> buffer_infos(
          builder.m_uniform_buffers.size());
      size_t current_binding{0};
      for (size_t j = 0; j < m_uniform_buffers[i].size();
           j++, current_binding++) {
        vk::DescriptorBufferInfo bi;
        bi.buffer = m_uniform_buffers[i][j].get_handle();
        bi.offset = 0;
        bi.range = builder.m_uniform_buffers[j].initial_state.size();
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

      m_context.get_device().updateDescriptorSets(writes, nullptr);
    }
  }
}

void Shader::_update_uniform_buffer(const vulkan::RenderCall &render_call,
                                    const void *data, const size_t data_size,
                                    const size_t index) {
  m_uniform_buffers[render_call.get_swap_chain_image_index()][index].set_data(
      data, data_size);
}
} // namespace core