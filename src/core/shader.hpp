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
class Shader {
public:
  friend class Builder;
  class Builder {
  public:
    friend class Shader;

    Builder();

    Shader build(const vulkan::Context &context, const Settings &settings);

    inline Builder &vertex(std::filesystem::path vertex_path) {
      m_vertex_code = _read_spv_file(std::move(vertex_path));
      return *this;
    }
    inline Builder &fragment(std::filesystem::path fragment_path) {
      m_fragment_code = _read_spv_file(std::move(fragment_path));
      return *this;
    }
    template <typename T>
    inline Builder &uniform_buffer(vk::ShaderStageFlags shader_stage,
                                   const T &initial_state) {
      std::vector<uint8_t> init(sizeof(T));
      memcpy(init.data(), &initial_state, init.size());

      m_uniform_buffers.emplace_back(
          UniformBufferInfo{std::move(init), shader_stage});

      return *this;
    }
    template <typename T> inline Builder &vertex_attribute() {
      m_vertex_attributes.emplace_back(
          VertexAttributeInfo{vulkan::vertex_attribute_format<T>, sizeof(T)});
      return *this;
    }

    inline Builder &texture() {
      m_texture_count++;
      return *this;
    }

  private:
    struct UniformBufferInfo {
      const std::vector<uint8_t> initial_state;
      const vk::ShaderStageFlags shader_stage;
    };
    struct VertexAttributeInfo {
      const vk::Format format;
      const size_t size;
    };

    static std::vector<uint8_t> _read_spv_file(std::filesystem::path file_name);

    std::vector<uint8_t> m_vertex_code;
    std::vector<uint8_t> m_fragment_code;
    std::vector<UniformBufferInfo> m_uniform_buffers;
    std::vector<VertexAttributeInfo> m_vertex_attributes;
    size_t m_texture_count;
  };

  Shader(Shader &&rhs);
  ~Shader();

  template <typename T>
  inline void update_uniform_buffer(const vulkan::RenderCall &render_call,
                                    const T &data, const size_t index = 0) {
    _update_uniform_buffer(render_call, &data, sizeof(T), index);
  }
  void set_texture(const Texture &texture, const size_t index = 0);

  void bind(const vulkan::RenderCall &render_call);

private:
  struct TextureWrite {
    std::set<uint32_t> image_indices;
    vk::DescriptorImageInfo image_info;
    vk::WriteDescriptorSet write;
  };

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
  std::vector<std::vector<vulkan::Buffer>> m_uniform_buffers;
  vk::DescriptorSetLayout m_descriptor_layout;
  vk::DescriptorPool m_descriptor_pool;
  std::vector<vk::DescriptorSet> m_descriptor_sets;

  std::map<size_t, TextureWrite> m_texture_writes_to_perform;

  const vulkan::Context &m_context;
};

} // namespace core