#include "ray.hpp"
#include <algorithm>

namespace physics {

float face_min(const float v1, const float v2, const Ray::Face f1,
               const Ray::Face f2, Ray::Face &rf) {
  if (v1 < v2) {
    rf = f1;
    return v1;
  }

  rf = f2;
  return v2;
}

float face_max(const float v1, const float v2, const Ray::Face f1,
               const Ray::Face f2, Ray::Face &rf) {
  if (v1 > v2) {
    rf = f1;
    return v1;
  }

  rf = f2;
  return v2;
}

float Ray::cast(const AABB &aabb, Ray::Face &face) const {
  const auto aabb_max(aabb.max());
  // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter3/raycast_aabb.html
  const auto t_x_min{(aabb.min().x - origin.x) / direction.x};
  const auto t_x_max{(aabb_max.x - origin.x) / direction.x};
  const auto t_y_min{(aabb.min().y - origin.y) / direction.y};
  const auto t_y_max{(aabb_max.y - origin.y) / direction.y};
  const auto t_z_min{(aabb.min().z - origin.z) / direction.z};
  const auto t_z_max{(aabb_max.z - origin.z) / direction.z};

  Face min_face, max_face, x_face, y_face, z_face;

  // biggest min value
  auto x_val{face_min(t_x_min, t_x_max, Face::LEFT, Face::RIGHT, x_face)};
  auto y_val{face_min(t_y_min, t_y_max, Face::BOTTOM, Face::TOP, y_face)};
  auto z_val{face_min(t_z_min, t_z_max, Face::FRONT, Face::BACK, z_face)};
  auto t_min{face_max(x_val, y_val, x_face, y_face, min_face)};
  t_min = face_max(t_min, z_val, min_face, z_face, min_face);

  // smallest max value
  x_val = face_max(t_x_min, t_x_max, Face::LEFT, Face::RIGHT, x_face);
  y_val = face_max(t_y_min, t_y_max, Face::BOTTOM, Face::TOP, y_face);
  z_val = face_max(t_z_min, t_z_max, Face::FRONT, Face::BACK, z_face);
  auto t_max{face_min(x_val, y_val, x_face, y_face, max_face)};
  t_max = face_min(t_max, z_val, max_face, z_face, max_face);

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
    face = max_face;
    return t_max;
  }

  face = min_face;
  return t_min;
}

} // namespace physics
