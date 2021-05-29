#include "settings.hpp"

namespace core {
Settings::Settings()
    : msaa_samples(vk::SampleCountFlagBits::e4), max_fps(60),
      window_width(1280), window_height(720) {}
} // namespace core