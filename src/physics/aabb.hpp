#pragma once
#include <glm/glm.hpp>

namespace physics {

class AABB {
public:
  AABB(const float x, const float y, const float z, const float w,
       const float h, const float d);

  bool collide(const AABB &other, float &x, float &y, float &z) const;

  glm::vec3 position;
  glm::vec3 dimensions;
};

} // namespace physics
