#include "resource_hodler.hpp"

namespace fonts {
#include <fonts/MisterPixelRegular.hpp>
}

namespace core {
void ResourceHodler::_load_all_fonts() {
  // Fonts
  load_font(debug_font_name, fonts::MisterPixelRegular_otf);
}
} // namespace core