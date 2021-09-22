#include "element.hpp"

namespace core {
namespace gui {

bool Element::_mouse_collides(const Window &window, const glm::vec2 &p,
                              const glm::uvec2 &s) {
  const auto &m(window.get_mouse().screen_position);
  const auto hw{static_cast<float>(s.x) / 2.0f};
  const auto hh{static_cast<float>(s.y) / 2.0f};

  return m.x > (p.x - hw) && m.x < (p.x + hw) && m.y > (p.y - hh) &&
         m.y < (p.y + hh);
}

} // namespace gui
} // namespace core