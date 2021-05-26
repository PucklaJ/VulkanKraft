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
    Builder();

    Shader build(const vulkan::Context &context);

    Builder &vertex(std::filesystem::path vertex_path);
    Builder &fragment(std::filesystem::path fragment_path);
    Builder &add_uniform_buffer(vk::ShaderStageFlags shader_stage);

  private:
    std::optional<std::filesystem::path> m_vertex_path;
    std::optional<std::filesystem::path> m_fragment_path;
    std::vector<vk::ShaderStageFlags> m_uniform_buffers;
  };

  class MatrixData {
  public:
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  ~Shader();

  void update_uniform_buffer(const MatrixData &data, const size_t index = 0);
  void bind();

private:
  static std::vector<char> _read_spv_file(std::filesystem::path file_name);

  Shader(const vulkan::Context &context, std::filesystem::path vertex_path,
         std::filesystem::path fragment_path,
         std::vector<vk::ShaderStageFlags> uniform_buffers);

  void _create_graphics_pipeline(
      const vulkan::Context &context, std::filesystem::path vertex_path,
      std::filesystem::path fragment_path,
      const std::vector<vk::ShaderStageFlags> &uniform_buffers);
  void _create_descriptor_pool(
      const vulkan::Context &context,
      const std::vector<vk::ShaderStageFlags> &uniform_buffers);
  void
  _create_uniform_buffers(const vulkan::Context &context,
                          std::vector<vk::ShaderStageFlags> uniform_buffers);
  void _create_descriptor_sets(const vulkan::Context &context);

  std::unique_ptr<vulkan::GraphicsPipeline> m_pipeline;
  std::vector<std::vector<vulkan::Buffer>> m_uniform_buffers;
  vk::DescriptorSetLayout m_descriptor_layout;
  vk::DescriptorPool m_descriptor_pool;
  std::vector<vk::DescriptorSet> m_descriptor_sets;

  const vulkan::Context &m_context;
};

} // namespace core