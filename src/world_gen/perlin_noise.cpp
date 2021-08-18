#include "perlin_noise.hpp"
#include "../core/math.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace world_gen {
PerlinNoise::PerlinNoise() { seed(1); }

void PerlinNoise::seed(const size_t seed) {
  for (size_t i = 0; i < noise_size; i++) {
    m_p[i] = i;
  }

  std::mt19937_64 random_engine;
  random_engine.seed(seed);

  std::shuffle(m_p.begin(), m_p.end() - 1, random_engine);
  m_p.back() = m_p[noise_size - 1];
}

float PerlinNoise::get(const float x, const float y, const float _freq,
                       const int oct) const {
  // Fractal Brownian Motion (FBM)

  float sum{0.0f};
  float freq{_freq};
  float amp{1.0f};

  for (int i = 0; i < oct; i++) {
    sum += _get(x, y, freq) * amp;
    freq *= 2.0f;
    amp /= 2.0f;
  }

  return _clamp(sum, -1.0f, 1.0f);
}

float PerlinNoise::_get(float x, float y, const float freq) const {
  x = core::math::abs(x + noise_offset);
  y = core::math::abs(y + noise_offset);
  x *= freq;
  y *= freq;

  const auto X{static_cast<size_t>(floorf(x)) % noise_size};
  const auto Y{static_cast<size_t>(floorf(y)) % noise_size};
  const auto xf{x - floorf(x)};
  const auto yf{y - floorf(y)};

#ifndef NDEBUG
  assert(X < noise_size);
  assert(Y < noise_size);
#endif

  const glm::vec2 top_right(xf - 1.0f, yf - 1.0f), top_left(xf, yf - 1.0f),
      bottom_right(xf - 1.0f, yf), bottom_left(xf, yf);

  const auto value_top_right{m_p[(m_p[X + 1] + Y + 1) % (noise_size + 1)]};
  const auto value_top_left{m_p[(m_p[X] + Y + 1) % (noise_size + 1)]};
  const auto value_bottom_right{m_p[(m_p[X + 1] + Y) % (noise_size + 1)]};
  const auto value_bottom_left{m_p[(m_p[X] + Y) % (noise_size + 1)]};

  const auto dot_top_right{
      glm::dot(top_right, _get_constant_vector(value_top_right))};
  const auto dot_top_left{
      glm::dot(top_left, _get_constant_vector(value_top_left))};
  const auto dot_bottom_right{
      glm::dot(bottom_right, _get_constant_vector(value_bottom_right))};
  const auto dot_bottom_left{
      glm::dot(bottom_left, _get_constant_vector(value_bottom_left))};

  const auto u{_fade(xf)};
  const auto v{_fade(yf)};
  const auto result{_lerp(u, _lerp(v, dot_bottom_left, dot_top_left),
                          _lerp(v, dot_bottom_right, dot_top_right))};

  return result;
}

} // namespace world_gen
