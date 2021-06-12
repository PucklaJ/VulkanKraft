#include "settings.hpp"
#include <glm/glm.hpp>

namespace core {
Settings::Settings()
    : msaa_samples(vk::SampleCountFlagBits::e4), max_fps(60),
      window_width(1280), window_height(720),
      field_of_view(glm::radians(70.0f)), render_distance(2) {}
} // namespace core