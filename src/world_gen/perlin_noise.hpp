#pragma once
#include <array>
#include <glm/glm.hpp>

namespace world_gen {
class PerlinNoise {
public:
  PerlinNoise();

  float get(float x, float y, const float freq = 0.005f) const;

private:
  static inline glm::vec2 _get_constant_vector(const int v) {
    const auto h{v % 4};

    switch (h) {
    case 0:
      return glm::vec2(1.0f, 1.0f);
    case 1:
      return glm::vec2(-1.0f, 1.0f);
    case 2:
      return glm::vec2(-1.0f, -1.0f);
    default:
      return glm::vec2(1.0f, -1.0f);
    }
  }

  template <typename S, typename T>
  static inline auto _lerp(const S &t, const T &a1, const T &a2) {
    return a1 + t * (a2 - a1);
  }

  template <typename S> static inline auto _fade(const S &t) {
    return ((static_cast<S>(6) * t - static_cast<S>(15)) * t +
            static_cast<S>(10)) *
           t * t * t;
  }

  std::array<int, 257> m_p;
};
} // namespace world_gen
