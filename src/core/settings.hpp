#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
class Settings {
public:
  Settings();

  vk::SampleCountFlagBits msaa_samples;
  size_t max_fps;
};
} // namespace core