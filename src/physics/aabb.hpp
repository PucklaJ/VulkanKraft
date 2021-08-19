#pragma once
#include <glm/glm.hpp>

namespace physics {

static float _temp;

class AABB {
public:
  AABB();
  AABB(const float x, const float y, const float z, const float w,
       const float h, const float d);

  bool collide(const AABB &other, float &x = _temp, float &y = _temp,
               float &z = _temp) const;

  constexpr const glm::vec3 &min() const { return position; }
  constexpr glm::vec3 &min() { return position; }
  constexpr glm::vec3 max() const { return position + dimensions; }

  glm::vec3 position;
  glm::vec3 dimensions;
};

} // namespace physics
