#pragma once
#include "settings.hpp"
#include "texture.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/graphics_pipeline.hpp"
#include "vulkan/vertex.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace core {
// A class representing a graphics pipeline and all its shenanigans
class Shader {
public:
  friend class Builder;

  // A class used to create a shader
  class Builder {
  public:
    friend class Shader;

    Builder();

    // Build the shader using previously set configurations
    Shader build(const vulkan::Context &context, const Settings &settings);

    // Set the shader code of the vertex shader
    template <size_t S>
    inline Builder &vertex(const std::array<uint8_t, S> &spv_data) {
      m_vertex_code = {spv_data.data(), S};
      return *this;
    }
    // Set the shader code of the fragment shader
    template <size_t S>
    inline Builder &fragment(const std::array<uint8_t, S> &spv_data) {
      m_fragment_code = {spv_data.data(), S};
      return *this;
    }
    // Add an uniform buffer and what should be its initial data (This also
    // determines the size of the buffer)
    template <typename T>
    inline Builder &uniform_buffer(vk::ShaderStageFlags shader_stage,
                                   const T &initial_state) {
      std::vector<uint8_t> init(sizeof(T));
      memcpy(init.data(), &initial_state, init.size());

      m_uniform_buffers.emplace_back(
          UniformBufferInfo{std::move(init), shader_stage});

      return *this;
    }

    // Add a vertex attribute of the given type T
    template <typename T> inline Builder &vertex_attribute() {
      m_vertex_attributes.emplace_back(
          VertexAttributeInfo{vulkan::vertex_attribute_format<T>, sizeof(T)});
      return *this;
    }

    // Add a static texture (A texture which is the same during one render call)
    inline Builder &texture() {
      m_texture_count++;
      return *this;
    }

    // Add a dynamic texture (A texture which can be dynamically changed during
    // one render call)
    // max_sets ..... Determines how much different textures can be used for
    // this dynamic texture
    inline Builder &dynamic_texture(const uint32_t max_sets) {
      m_dynamic_textures.emplace_back(max_sets);
      return *this;
    }

    // Enable alpha blending for the shader
    inline Builder &alpha_blending() {
      m_alpha_blending = true;
      return *this;
    }

    template <typename T>
    inline Builder &push_constant(vk::ShaderStageFlags shader_stage,
                                  const T &data) {
      m_push_constants.emplace_back(
          shader_stage, m_current_push_constant_offset, sizeof(data));
      m_current_push_constant_offset += sizeof(data);
      return *this;
    }

    template <typename T>
    inline Builder &push_constant(vk::ShaderStageFlags shader_stage) {
      m_push_constants.emplace_back(shader_stage,
                                    m_current_push_constant_offset, sizeof(T));
      m_current_push_constant_offset += sizeof(T);
      return *this;
    }

  private:
    // Stores the initial state and for what shader stage the uniform buffer
    // is used
    struct UniformBufferInfo {
      const std::vector<uint8_t> initial_state;
      const vk::ShaderStageFlags shader_stage;
    };
    // Stores what format a vertex attribute has and how big it is in bytes
    struct VertexAttributeInfo {
      const vk::Format format;
      const size_t size;
    };

    vulkan::SPVData m_vertex_code;
    vulkan::SPVData m_fragment_code;
    std::vector<UniformBufferInfo> m_uniform_buffers;
    std::vector<VertexAttributeInfo> m_vertex_attributes;
    size_t m_texture_count;
    std::vector<uint32_t> m_dynamic_textures;
    bool m_alpha_blending;
    std::vector<vk::PushConstantRange> m_push_constants;
    uint32_t m_current_push_constant_offset;
  };

  Shader(Shader &&rhs);
  ~Shader();

  // Update the given uniform buffer using the given data
  template <typename T>
  inline void update_uniform_buffer(const vulkan::RenderCall &render_call,
                                    const T &data, const size_t index = 0) {
    _update_uniform_buffer(render_call, &data, sizeof(T), index);
  }
  // Update the given static texture
  void set_texture(const Texture &texture, const size_t index = 0);
  // Update the given dynamic texture (requires a render call)
  void bind_dynamic_texture(const vulkan::RenderCall &render_call,
                            Texture &texture, const size_t index = 0) const;
  template <typename T>
  inline void set_push_constant(const vulkan::RenderCall &render_call,
                                const T &data, const size_t index = 0) const {
    render_call.set_push_constant(m_pipeline->get_layout(),
                                  m_push_constants[index].shader_stage,
                                  m_push_constants[index].offset, data);
  }

  // Bind the shader for all following draw calls
  void bind(const vulkan::RenderCall &render_call);

private:
  // Represents a write to the descriptor sets
  struct TextureWrite {
    // Which swap image indices need to be updated
    std::set<uint32_t> image_indices;
    vk::DescriptorImageInfo image_info;
    vk::WriteDescriptorSet write;
  };

  // Stores information about a push constant required to set the push constant
  struct PushConstantData {
    const vk::ShaderStageFlags shader_stage;
    const uint32_t offset;
  };

  // Make the constructor private so that only the Builder can create shaders
  Shader(const vulkan::Context &context, const Settings &settings,
         const Builder &builder);

  void _create_graphics_pipeline(const vulkan::Context &context,
                                 const Settings &settings,
                                 const Builder &builder);
  void _create_descriptor_pool(const vulkan::Context &context,
                               const Builder &builder);
  void _create_uniform_buffers(const vulkan::Context &context,
                               const Builder &builder);
  void _create_descriptor_sets(const vulkan::Context &context,
                               const Builder &builder);
  void _update_uniform_buffer(const vulkan::RenderCall &render_call,
                              const void *data, const size_t data_size,
                              const size_t index);

  std::unique_ptr<vulkan::GraphicsPipeline> m_pipeline;
  // Store unform buffers for every swap chain image
  std::vector<std::vector<vulkan::Buffer>> m_uniform_buffers;
  vk::DescriptorSetLayout m_descriptor_layout;
  std::vector<vk::DescriptorSetLayout> m_dynamic_textures_layout;
  vk::DescriptorPool m_descriptor_pool;
  std::vector<vk::DescriptorSet> m_descriptor_sets;
  std::vector<PushConstantData> m_push_constants;

  std::map<size_t, TextureWrite> m_texture_writes_to_perform;
  size_t m_min_dynamic_texture_binding_point;

  const vulkan::Context &m_context;
};

} // namespace core