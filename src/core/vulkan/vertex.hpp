#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
template <typename T> inline constexpr vk::Format vertex_attribute_format;
template <>
inline constexpr vk::Format vertex_attribute_format<glm::f32> =
    vk::Format::eR32Sfloat;
template <>
inline constexpr vk::Format vertex_attribute_format<glm::vec2> =
    vk::Format::eR32G32Sfloat;
template <>
inline constexpr vk::Format vertex_attribute_format<glm::vec3> =
    vk::Format::eR32G32B32Sfloat;
template <>
inline constexpr vk::Format vertex_attribute_format<glm::vec4> =
    vk::Format::eR32G32B32A32Sfloat;

class Vertex {
public:
  Vertex(glm::f64 pos_x, glm::f64 pos_y, glm::f64 pos_z)
      : position(pos_x, pos_y, pos_z) {}

  glm::vec3 position;
};

} // namespace vulkan
} // namespace core
