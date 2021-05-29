#include "settings.hpp"

namespace core {
Settings::Settings() : msaa_samples(vk::SampleCountFlagBits::e4) {}
} // namespace core