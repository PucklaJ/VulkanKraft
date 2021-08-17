#pragma once
#include "type.hpp"
#include <glm/glm.hpp>
#include <map>

namespace block {

class Server {
public:
  static constexpr bool block_is_solid(const Type type) {
    switch (type) {
    case Type::AIR:
      return false;
    default:
      return true;
    }
  }

  struct TextureCoordinates {
    glm::vec4 front;
    glm::vec4 back;
    glm::vec4 left;
    glm::vec4 right;
    glm::vec4 bot;
    glm::vec4 top;
  };

  Server();

  inline const TextureCoordinates &
  get_texture_coordinates(const Type type) const {
    return m_block_data.at(type).tex_coords;
  }

private:
  static constexpr float block_texture_size = 512.0f;
  static constexpr float block_pixel_size = 16.0f;

  // Returns the uv texture coordinates for the block at horizontal x position
  // and vertical y position
  static constexpr glm::vec4 _block_tex_coords(int x, int y) {
    constexpr auto ps2 = 0.0f;
    return glm::vec4(
        static_cast<float>(x) * block_pixel_size / block_texture_size + ps2,
        static_cast<float>(y) * block_pixel_size / block_texture_size + ps2,
        static_cast<float>(x + 1) * block_pixel_size / block_texture_size - ps2,
        static_cast<float>(y + 1) * block_pixel_size / block_texture_size -
            ps2);
  }

  struct BlockData {
    TextureCoordinates tex_coords;
  };

  std::map<Type, BlockData> m_block_data;
};

} // namespace block