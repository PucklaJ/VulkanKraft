#include "settings.hpp"

namespace core {
Settings::Settings() : msaa_samples(vk::SampleCountFlagBits::e4), max_fps(60) {}
} // namespace core