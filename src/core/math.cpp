#include "math.hpp"
#include <algorithm>

namespace core {
namespace math {
float Ray::cast(const AABB &aabb) const {
  // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter3/raycast_aabb.html
  const auto t1{(aabb.min.x - origin.x) / direction.x};
  const auto t2{(aabb.max.x - origin.x) / direction.x};
  const auto t3{(aabb.min.y - origin.y) / direction.y};
  const auto t4{(aabb.max.y - origin.y) / direction.y};
  const auto t5{(aabb.min.z - origin.z) / direction.z};
  const auto t6{(aabb.max.z - origin.z) / direction.z};

  // biggest min value
  const auto t_min{
      std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6))};
  // smallest max value
  const auto t_max{
      std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6))};

  // whole AABB is behind us
  if (t_max < 0.0f) {
    return -1.0f;
  }

  // ray doesn't intersect AABB
  if (t_min > t_max) {
    return -1.0f;
  }

  // ray is inside AABB
  if (t_min < 0.0f) {
    return t_max;
  }

  return t_min;
}
} // namespace math
} // namespace core
