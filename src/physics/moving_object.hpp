#pragma once
#include "aabb.hpp"
#include <glm/glm.hpp>

namespace physics {
class MovingObject {
public:
  friend class Server;

  MovingObject(const glm::vec3 &initial_position,
               const glm::vec3 &aabb_dimensions, const glm::vec3 &aabb_offset);

  glm::vec3 position;
  glm::vec3 velocity;

private:
  static constexpr auto gravity = glm::vec3(0.0f, -10.0f, 0.0f);

  void _compute_new_aabb_position(const float delta_time);
  void _compute_new_position();

  AABB m_aabb;

  const glm::vec3 m_aabb_offset;
};
} // namespace physics
