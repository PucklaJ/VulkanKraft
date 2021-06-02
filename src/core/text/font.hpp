#pragma once
#include <filesystem>
#ifndef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#endif
#include <string>
#include <vector>

namespace core {
namespace text {
class Font {
public:
  Font(std::filesystem::path font_file_name);

  std::vector<float> create_bitmap(std::wstring text_string,
                                   const float font_size,
                                   size_t &complete_width,
                                   size_t &complete_height);

private:
  struct FontBitmap {
    std::vector<uint8_t> pixels;
    size_t pixel_width;
    size_t pixel_height;
  };

  struct StringBitmap {
    std::vector<FontBitmap> bitmaps;
    std::vector<size_t> x_positions;
    std::vector<size_t> y_positions;
  };

  stbtt_fontinfo m_font_info;
  std::vector<uint8_t> m_font_file_buffer;
};
} // namespace text
} // namespace core