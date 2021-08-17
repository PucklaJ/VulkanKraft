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

// Returns the absolute value of t
template <typename T> static inline auto abs(const T &t) {
  return ((t < static_cast<T>(0)) * static_cast<T>(-2) + static_cast<T>(1)) * t;
}

} // namespace math
} // namespace core