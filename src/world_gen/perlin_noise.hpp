#pragma once
#include <array>
#include <glm/glm.hpp>

namespace world_gen {
// A perlin noise implementation used for the world generation
class PerlinNoise {
public:
  PerlinNoise();

  // Seed the noise and change the values you get
  void seed(const size_t seed);

  // Get a noise value
  // oct is used for Fractal Brownian Motion (FBM)
  // returns a value between -1 and 1
  float get(const float x, const float y, const float freq = 0.005f,
            const int oct = 8) const;

private:
  // The size of the m_p array. Defines how diverse the noise is
  static constexpr size_t noise_size = 1024;
  static constexpr float noise_offset = 0.0f;

  // Returns a different vector of the given 4 based on v
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

  // Clamps v between min and max
  template <typename T>
  static inline auto _clamp(const T &v, const T &min, const T &max) {
    return (v < min) * min + (v > max) * max + (v >= min && v <= max) * v;
  }

  // Linearly interpolates from a1 to a2 based on t
  // t == 1 --> a2; t == 0 --> a1
  template <typename S, typename T>
  static inline auto _lerp(const S &t, const T &a1, const T &a2) {
    return a1 + t * (a2 - a1);
  }

  // A smooth function used in the noise
  template <typename S> static inline auto _fade(const S &t) {
    return ((static_cast<S>(6) * t - static_cast<S>(15)) * t +
            static_cast<S>(10)) *
           t * t * t;
  }

  // Returns the absolute value of t
  template <typename T> static inline auto _abs(const T &t) {
    return ((t < static_cast<T>(0)) * static_cast<T>(-2) + static_cast<T>(1)) *
           t;
  }

  // An internally noise function without Fractal Brownian Motion (FBM)
  float _get(float x, float y, const float freq) const;

  std::array<size_t, noise_size + 1> m_p;
};
} // namespace world_gen
