#pragma once
#include "aabb.hpp"

namespace physics {
class Ray {
public:
  enum Face {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
  };

  float cast(const AABB &aabb, Face &face) const;

  glm::vec3 origin;
  glm::vec3 direction;
};
} // namespace physics
