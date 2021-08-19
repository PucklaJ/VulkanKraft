#include "moving_object.hpp"

namespace physics {
MovingObject::MovingObject(const glm::vec3 &initial_position,
                           const glm::vec3 &aabb_dimensions,
                           const glm::vec3 &aabb_offset)
    : position(initial_position), velocity(0.0f, 0.0f, 0.0f),
      m_aabb(0.0f, 0.0f, 0.0f, aabb_dimensions.x, aabb_dimensions.y,
             aabb_dimensions.z),
      m_aabb_offset(aabb_offset) {}

void MovingObject::_compute_new_aabb_position(const float delta_time) {
  // Compute force and gravity
  velocity += gravity * delta_time;
  position += velocity * delta_time;

  // Calculate position of AABB
  m_aabb.position = position + m_aabb_offset;
}

void MovingObject::_compute_new_position() {
  position = m_aabb.position - m_aabb_offset;
}
} // namespace physics
