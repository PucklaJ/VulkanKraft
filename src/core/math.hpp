#pragma once
#include <glm/glm.hpp>

namespace core {
namespace math {

// Returns the absolute value of t
template <typename T> static inline auto abs(const T &t) {
  return ((t < static_cast<T>(0)) * static_cast<T>(-2) + static_cast<T>(1)) * t;
}

} // namespace math
} // namespace core