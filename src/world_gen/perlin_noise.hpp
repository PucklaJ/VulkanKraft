#pragma once
#include <array>
#include <glm/glm.hpp>

namespace world_gen {
class PerlinNoise {
public:
  PerlinNoise();

  void seed(const size_t seed);

  float get(const float x, const float y, const float freq = 0.005f,
            const int oct = 8) const;

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

  template <typename T>
  static inline auto _clamp(const T &v, const T &min, const T &max) {
    return (v < min) * min + (v > max) * max + (v >= min && v <= max) * v;
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

  template <typename T> static inline auto _abs(const T &t) {
    return ((t < static_cast<T>(0)) * static_cast<T>(-2) + static_cast<T>(1)) *
           t;
  }

  float _get(float x, float y, const float freq) const;

  std::array<int, 257> m_p;
};
} // namespace world_gen
