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

  // Casts a this ray onto the aabb and returns at what distance on the ray it
  // intersects with the aabb. If the ray doesn't intersect the returned value
  // is negative.
  // face ..... What face of the AABB is hit
  float cast(const AABB &aabb, Face &face) const;

  glm::vec3 origin;
  glm::vec3 direction;
};
} // namespace physics
