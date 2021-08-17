#include "../aabb.hpp"
#include <cassert>

int main(int args, char *argv[]) {
  const physics::AABB aabb1(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
  const physics::AABB aabb2(2.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f);
  const physics::AABB aabb3(-2.0f, 0.0f, -3.0f, 2.25f, 1.0f, 3.25f);

  float x, y, z;
  assert(aabb1.collide(aabb2, x, y, z) == false);
  assert(aabb1.collide(aabb3, x, y, z) == true);
  assert(x == -0.25f);
  assert(y == 0.0f);
  assert(z == -0.25f);
  assert(aabb3.collide(aabb2, x, y, z) == false);

  return 0;
}