#include "aabb.hpp"
#include <algorithm>

namespace physics {
AABB::AABB(const float x, const float y, const float z, const float w,
           const float h, const float d)
    : position(x, y, z), dimensions(w, h, d) {}

bool AABB::collide(const AABB &other, float &x, float &y, float &z) const {
  const auto collides{other.position.x < position.x + dimensions.x &&
                      other.position.x + other.dimensions.x > position.x &&
                      other.position.y < position.y + dimensions.y &&
                      other.position.y + other.dimensions.y > position.y &&
                      other.position.z < position.z + dimensions.z &&
                      other.position.z + other.dimensions.z > position.z};

  if (!collides)
    return false;

  {
    const auto min_in{other.position.x > position.x};
    const auto max_in{other.position.x + other.dimensions.x <
                      position.x + dimensions.x};

    const auto min_left_x{(other.position.x - position.x) *
                          (min_in * 1.0f - !min_in * 1.0f)};
    const auto min_right_x{position.x + dimensions.x - other.position.x};
    const auto max_left_x{other.position.x + other.dimensions.x - position.x};
    const auto max_right_x{
        (position.x + dimensions.x - other.position.x - other.dimensions.x) *
        (max_in * 1.0f - !max_in * 1.0f)};

    const auto min_x{std::min(min_left_x, min_right_x)};
    const auto max_x{std::min(max_left_x, max_right_x)};

    x = min_in * min_x + max_in * -max_x;
  }

  {
    const auto min_in{other.position.y > position.y};
    const auto max_in{other.position.y + other.dimensions.y <
                      position.y + dimensions.y};

    const auto min_left_y{other.position.y - position.y};
    const auto min_right_y{position.y + dimensions.y - other.position.y};
    const auto max_left_y{other.position.y + other.dimensions.y - position.y};
    const auto max_right_y{position.y + dimensions.y - other.position.y -
                           other.dimensions.y};

    const auto min_y{std::min(min_left_y, min_right_y)};
    const auto max_y{std::min(max_left_y, max_right_y)};

    y = min_in * min_y + max_in * -max_y;
  }

  {
    const auto min_in{other.position.z > position.z};
    const auto max_in{other.position.z + other.dimensions.z <
                      position.z + dimensions.z};

    const auto min_left_z{(other.position.z - position.z) *
                          (min_in * 1.0f - !min_in * 1.0f)};
    const auto min_right_z{position.z + dimensions.z - other.position.z};
    const auto max_left_z{other.position.z + other.dimensions.z - position.z};
    const auto max_right_z{
        (position.z + dimensions.z - other.position.z - other.dimensions.z) *
        (max_in * 1.0f - !max_in * 1.0f)};

    const auto min_z{std::min(min_left_z, min_right_z)};
    const auto max_z{std::min(max_left_z, max_right_z)};

    z = min_in * min_z + max_in * -max_z;
  }

  return true;
}
} // namespace physics
