#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "../exception.hpp"
#include "font.hpp"
#include <fstream>

namespace core {
namespace text {
Font::Font(std::filesystem::path font_file_name) {
  std::ifstream file;
  file.open(font_file_name, std::ios_base::binary | std::ios_base::ate);
  if (file.fail()) {
    throw VulkanKraftException("failed to load font file: " +
                               font_file_name.string());
  }
  const auto file_size = file.tellg();
  file.seekg(0);
  m_font_file_buffer.resize(file_size);
  file.read(reinterpret_cast<char *>(m_font_file_buffer.data()), file_size);
  file.close();

  if (stbtt_InitFont(&m_font_info, m_font_file_buffer.data(), 0) == 0) {
    throw VulkanKraftException("Failed to initialise font file: " +
                               font_file_name.string());
  }
}

std::vector<uint8_t> Font::create_bitmap(std::wstring text_string,
                                         const float font_size) {
  // Initialise all font values
  const auto scale = stbtt_ScaleForPixelHeight(&m_font_info, font_size);
  int ascent, descent, line_gap;
  stbtt_GetFontVMetrics(&m_font_info, &ascent, &descent, &line_gap);
  ascent = static_cast<int>(ascent * scale);
  descent = static_cast<int>(descent * scale);
  line_gap = static_cast<int>(line_gap * scale);

  // Stores the bitmaps of all characters
  StringBitmap all_characters;
  all_characters.bitmaps.reserve(text_string.length());
  all_characters.x_positions.reserve(text_string.length());
  all_characters.y_positions.reserve(text_string.length());

  // Loop over all characters
  auto current_ascent = ascent;
  float current_x = 0.0f;
  for (size_t i = 0; i < text_string.length(); i++) {
    // If we've found a new line we should advance to the next line
    if (text_string[i] == L'\n') {
      current_x = 0.0f;
      current_ascent += ascent - descent + line_gap;
      continue;
    }

    // Get all metrics from the current character
    int advance, lsb, x0, y0, x1, y1;
    const auto x_shift = current_x - floorf(current_x);
    stbtt_GetCodepointHMetrics(&m_font_info, text_string[i], &advance, &lsb);
    stbtt_GetCodepointBitmapBoxSubpixel(&m_font_info, text_string[i], scale,
                                        scale, x_shift, 0, &x0, &y0, &x1, &y1);

    // Allocate memory for the bitmap
    FontBitmap fb;
    fb.pixel_width = x1 - x0;
    fb.pixel_height = y1 - y0;
    fb.pixels.resize(fb.pixel_width * fb.pixel_height);
    // Create the bitmap
    stbtt_MakeCodepointBitmapSubpixel(
        &m_font_info, fb.pixels.data(), fb.pixel_width, fb.pixel_height,
        fb.pixel_width, scale, scale, x_shift, 0, text_string[i]);

    // Insert the bitmap into all_characters
    all_characters.bitmaps.emplace_back(std::move(fb));
    all_characters.x_positions.emplace_back(
        std::max(static_cast<int>(current_x + x0), 0));
    all_characters.y_positions.emplace_back(
        std::max(static_cast<int>(current_ascent + y0), 0));

    // Advance the x position
    if (current_x + x0 < 0) {
      current_x -= x0;
    }
    current_x += advance * scale;
    // Add kerning
    if (i != text_string.length() - 1) {
      current_x +=
          scale * stbtt_GetCodepointKernAdvance(&m_font_info, text_string[i],
                                                text_string[i + 1]);
    }
  }

  size_t complete_width = 0;
  size_t complete_height = 0;
  size_t min_y = -1;
  // Get the max width and max height
  for (size_t i = 0; i < all_characters.bitmaps.size(); i++) {
    const auto new_height =
        all_characters.y_positions[i] + all_characters.bitmaps[i].pixel_height;
    const auto new_width =
        all_characters.x_positions[i] + all_characters.bitmaps[i].pixel_width;

    if (new_height > complete_height)
      complete_height = new_height;
    if (new_width > complete_width) {
      complete_width = new_width;
    }
    if (all_characters.y_positions[i] < min_y) {
      min_y = all_characters.y_positions[i];
    }
  }
  complete_height -= min_y;

  // Allocate memory for the complete string
  std::vector<uint8_t> complete_string(complete_width * complete_height);
  // Write all bitmaps into the complete string
  for (size_t c = 0; c < all_characters.bitmaps.size(); c++) {
    const auto x = all_characters.x_positions[c];
    const auto y = all_characters.y_positions[c] - min_y;
    for (size_t i = 0; i < all_characters.bitmaps[c].pixel_width; i++) {
      for (size_t j = 0; j < all_characters.bitmaps[c].pixel_height; j++) {
        // Clip when y gets below 0
        if (static_cast<long>(j) + y < 0) {
          continue;
        }
        const auto complete_index = (i + x) + (j + y) * complete_width;
        const auto bitmap_index = i + j * all_characters.bitmaps[c].pixel_width;

        complete_string[complete_index] =
            all_characters.bitmaps[c].pixels[bitmap_index];
      }
    }
  }

  return complete_string;
}
} // namespace text
} // namespace core
