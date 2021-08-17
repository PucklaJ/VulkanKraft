#pragma once
#include <glm/glm.hpp>

namespace physics {

static float _temp;

class AABB {
public:
  AABB(const float x, const float y, const float z, const float w,
       const float h, const float d);

  bool collide(const AABB &other, float &x = _temp, float &y = _temp,
               float &z = _temp) const;

  glm::vec3 position;
  glm::vec3 dimensions;
};

} // namespace physics
