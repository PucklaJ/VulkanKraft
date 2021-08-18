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
// Represents a TTF or OTF font file
class Font {
public:
  // Open a font from a file
  Font(std::filesystem::path font_file_name);
  // Open a font from a buffer
  Font(const uint8_t *font_file_buffer);

  // Creates a bitmap texture using the given arguments
  // complete_width, complete_height .... the dimensions of the texture
  // returns the newly cerated bitmap in an R32Sfloat format
  std::vector<float> create_bitmap(std::wstring text_string,
                                   const float font_size,
                                   size_t &complete_width,
                                   size_t &complete_height);

private:
  // Represents a bitmap consisting of one character
  class CharBitmap {
  public:
    CharBitmap(stbtt_fontinfo *font_info, const float scale,
               const float x_shift, const wchar_t character);

    // The raw pixel data
    std::vector<uint8_t> pixels;
    // The width of the bitmap in pixels
    size_t pixel_width;
    // The height of the bitmap in pixels
    size_t pixel_height;
    // The x offset of the cursor when drawing this bitmap
    int x0;
    // The y offset of the cursor when drawing this bitmap
    int y0;
    // An additional x advance of the cursor
    int advance;
  };

  // Represents an whole string consisting of char bitmaps
  struct StringBitmap {
    std::vector<CharBitmap *> bitmaps;
    // The x position of every bitmap
    std::vector<size_t> x_positions;
    // The y position of every bitmap
    std::vector<size_t> y_positions;
  };

  stbtt_fontinfo m_font_info;
  // Used to store a font file if the font has been loaded from a file
  std::vector<uint8_t> m_font_file_buffer;

  // Stores a CharBitmap for every character of every font size
  std::map<std::pair<float, wchar_t>, std::unique_ptr<CharBitmap>>
      m_bitmap_cache;
};
} // namespace text
} // namespace core