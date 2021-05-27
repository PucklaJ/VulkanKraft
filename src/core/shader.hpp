#pragma once
#include "vulkan/buffer.hpp"
#include "vulkan/graphics_pipeline.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace core {
class Shader {
public:
  friend class Builder;
  class Builder {
  public:
    friend class Shader;

    Builder();

    Shader build(const vulkan::Context &context);

    inline Builder &vertex(std::filesystem::path vertex_path) {
      m_vertex_code = _read_spv_file(std::move(vertex_path));
      return *this;
    }
    inline Builder &fragment(std::filesystem::path fragment_path) {
      m_fragment_code = _read_spv_file(std::move(fragment_path));
      return *this;
    }
    template <typename T>
    inline Builder &add_uniform_buffer(vk::ShaderStageFlags shader_stage,
                                       const T &initial_state) {
      std::vector<uint8_t> init(sizeof(T));
      memcpy(init.data(), &initial_state, init.size());

      m_uniform_buffers.emplace_back(
          UniformBufferInfo{std::move(init), shader_stage});

      return *this;
    }

  private:
    struct UniformBufferInfo {
      const std::vector<uint8_t> initial_state;
      const vk::ShaderStageFlags shader_stage;
    };

    static std::vector<uint8_t> _read_spv_file(std::filesystem::path file_name);

    std::vector<uint8_t> m_vertex_code;
    std::vector<uint8_t> m_fragment_code;
    std::vector<UniformBufferInfo> m_uniform_buffers;
  };

  ~Shader();

  template <typename T>
  inline void update_uniform_buffer(const vulkan::RenderCall &render_call,
                                    const T &data, const size_t index = 0) {
    _update_uniform_buffer(render_call, &data, sizeof(T), index);
  }
  void bind(const vulkan::RenderCall &render_call);

private:
  Shader(const vulkan::Context &context, std::vector<uint8_t> vertex_code,
         std::vector<uint8_t> fragment_code,
         std::vector<Builder::UniformBufferInfo> uniform_buffers);

  void _create_graphics_pipeline(
      const vulkan::Context &context, std::vector<uint8_t> vertex_code,
      std::vector<uint8_t> fragment_code,
      const std::vector<Builder::UniformBufferInfo> &uniform_buffers);
  void _create_descriptor_pool(
      const vulkan::Context &context,
      const std::vector<Builder::UniformBufferInfo> &uniform_buffers);
  void _create_uniform_buffers(
      const vulkan::Context &context,
      const std::vector<Builder::UniformBufferInfo> &uniform_buffers);
  void _create_descriptor_sets(
      const vulkan::Context &context,
      std::vector<Builder::UniformBufferInfo> uniform_buffers);
  void _update_uniform_buffer(const vulkan::RenderCall &render_call,
                              const void *data, const size_t data_size,
                              const size_t index);

  std::unique_ptr<vulkan::GraphicsPipeline> m_pipeline;
  std::vector<std::vector<vulkan::Buffer>> m_uniform_buffers;
  vk::DescriptorSetLayout m_descriptor_layout;
  vk::DescriptorPool m_descriptor_pool;
  std::vector<vk::DescriptorSet> m_descriptor_sets;

  const vulkan::Context &m_context;
};

} // namespace core