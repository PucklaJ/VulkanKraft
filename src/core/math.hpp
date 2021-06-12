#pragma once
#include <glm/glm.hpp>

namespace core {
namespace math {
class AABB {
public:
  glm::vec3 min;
  glm::vec3 max;
};

class Ray {
public:
  float cast(const AABB &aabb) const;

  glm::vec3 origin;
  glm::vec3 direction;
};
} // namespace math
} // namespace core