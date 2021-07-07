#pragma once
#include "type.hpp"
#include <glm/glm.hpp>
#include <map>

namespace block {

class Server {
public:
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
  struct BlockData {
    TextureCoordinates tex_coords;
  };

  std::map<Type, BlockData> m_block_data;
};

} // namespace block