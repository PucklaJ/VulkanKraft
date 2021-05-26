#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace core {
namespace vulkan {
class Vertex {
public:
  static inline vk::VertexInputBindingDescription get_binding_description() {
    vk::VertexInputBindingDescription d;
    d.binding = 0;
    d.stride = sizeof(Vertex);
    d.inputRate = vk::VertexInputRate::eVertex;
    return d;
  }

  static inline std::array<vk::VertexInputAttributeDescription, 1>
  get_attribute_description() {
    std::array<vk::VertexInputAttributeDescription, 1> ad;

    ad[0].binding = 0;
    ad[0].location = 0;
    ad[0].format = vk::Format::eR32G32B32Sfloat;
    ad[0].offset = offsetof(Vertex, position);

    return ad;
  }

  Vertex(glm::f64 pos_x, glm::f64 pos_y, glm::f64 pos_z)
      : position(pos_x, pos_y, pos_z) {}

  glm::vec3 position;
};
} // namespace vulkan
} // namespace core
