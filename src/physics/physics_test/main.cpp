#include "../../core/math.hpp"
#include "../aabb.hpp"
#include <cassert>

inline bool almost(const float v1, const float v2) {
  return core::math::abs(v1 - v2) < 1e-5;
}

int main(int args, char *argv[]) {
  const physics::AABB aabb1(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
  const physics::AABB aabb2(2.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f);
  const physics::AABB aabb3(-2.0f, 0.0f, -3.0f, 2.25f, 1.0f, 3.25f);

  const physics::AABB aabb4(-20.0f, 30.0f, -60.0f, 1.0f, 1.0f, 1.0f);
  const physics::AABB aabb5(-20.6f, 30.0f, -60.5f, 0.7f, 1.8f, 0.7f);

  float x, y, z;
  assert(aabb1.collide(aabb2, x, y, z) == false);
  assert(aabb1.collide(aabb3, x, y, z) == true);
  assert(almost(x, -0.25f));
  assert(almost(y, 0.0f));
  assert(almost(z, -0.25f));
  assert(aabb3.collide(aabb2, x, y, z) == false);
  assert(aabb4.collide(aabb5, x, y, z) == true);
  assert(almost(x, -0.1f));
  assert(almost(y, 0.0f));
  assert(almost(z, -0.2f));

  return 0;
}