#pragma once
#include <filesystem>
#ifndef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#endif
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace core {
namespace text {
class Font {
public:
  Font(std::filesystem::path font_file_name);
  Font(const uint8_t *font_file_buffer);

  std::vector<float> create_bitmap(std::wstring text_string,
                                   const float font_size,
                                   size_t &complete_width,
                                   size_t &complete_height);

private:
  class CharBitmap {
  public:
    CharBitmap(stbtt_fontinfo *font_info, const float scale,
               const float x_shift, const wchar_t character);

    std::vector<uint8_t> pixels;
    size_t pixel_width;
    size_t pixel_height;
    int x0;
    int y0;
    int advance;
  };

  struct StringBitmap {
    std::vector<CharBitmap *> bitmaps;
    std::vector<size_t> x_positions;
    std::vector<size_t> y_positions;
  };

  stbtt_fontinfo m_font_info;
  std::vector<uint8_t> m_font_file_buffer;

  // Stores a CharBitmap for every character of every font size
  std::map<std::pair<float, wchar_t>, std::unique_ptr<CharBitmap>>
      m_bitmap_cache;
};
} // namespace text
} // namespace core