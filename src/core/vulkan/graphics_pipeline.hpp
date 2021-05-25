#pragma once
#include "context.hpp"
#include <optional>
#include <vector>

namespace core {
namespace vulkan {
class GraphicsPipeline {
public:
  GraphicsPipeline(Context &context, std::vector<char> vertex_code,
                   std::vector<char> fragment_code);
  ~GraphicsPipeline();

  void bind();
  void recreate();

private:
  static constexpr char _shader_function_name[] = "main";

  static vk::ShaderModule _create_shader_module(const vk::Device &device,
                                                std::vector<char> shader_code);

  void _create_handle(std::vector<char> vertex_code = {},
                      std::vector<char> fragment_code = {});
  void _destroy(const bool everything = true);

  vk::Pipeline m_handle;
  std::optional<vk::PipelineLayout> m_layout;
  std::optional<vk::ShaderModule> m_vertex_module;
  std::optional<vk::ShaderModule> m_fragment_module;

  Context &m_context;
};
} // namespace vulkan
} // namespace core